// Antonio Hernandez Ruiz

// standard libraries
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// user libraries
#include "Agent.h"
#include "Ascii.h"
#include "Message.h"
#include "BattleBoats.h"
#include "BOARD.h"
#include "Buttons.h"
#include "CircularBuffer.h"
#include "FieldOled.h"
#include "Oled.h"
#include "OledDriver.h"
#include "Uart1.h"
#include "Negotiation.h"
#include "Field.h"

#define RANDOM 0xFFFF
#define GAMEOVER 0b00000000

struct Agent {
    AgentState State;
    NegotiationData place;
    NegotiationData hash;
    Field own_field;
    Field opponent;
    Message mssg;
};
// variables to test and have add values to
static struct Agent player;
static char *test_net1;
static char *test_net2;
static char *test_net3;
static char *test_net4;
static char *test_net5;
static char *test_net6;
static char *test_net7;
static FieldOledTurn player_turn;

// counter to store the amount of turns or what turn the game is on
static int counter = 0;

// setting the start state of the the Agent SM
// and setting the turn counter to 0 since it is
// just the beginning
// resetting whose turn it is at the start too
void AgentInit(void) {
    
    player.State = AGENT_STATE_START;
    counter = 0;
    player_turn = FIELD_OLED_TURN_NONE;
    

}
// just a big switch statement that accounts for all the different states that 
// the state machine can be in
Message AgentRun(BB_Event event) {
    switch (event.type) {
        // the first state checks for any errors that can arise in testing 
        // and prints out messages accordingly 
        case BB_EVENT_ERROR:
            switch (event.param0) {
                // first if a bad checksum is raised then print the correct message
                case BB_ERROR_BAD_CHECKSUM:
                    test_net1 = "Incorrect checksum";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(test_net1);
                    OledUpdate();
                    break;
                // checking for invalid messages and printing a message
                case BB_ERROR_INVALID_MESSAGE_TYPE:
                    test_net5 = "wrong message type";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(test_net5);
                    OledUpdate();
                    break;    
                // if the payload length is exceeded print the correct message
                case BB_ERROR_PAYLOAD_LEN_EXCEEDED:
                    test_net2 = "wrong payload len";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(test_net2);
                    OledUpdate();
                    break;
                // if the message can't be parsed print the correct message    
                case BB_ERROR_MESSAGE_PARSE_FAILURE:
                    test_net6 = "failed to parse mess.";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(test_net6);
                    OledUpdate();
                    break;    
                // if the checksum length is exceeded print the correct message
                case BB_ERROR_CHECKSUM_LEN_EXCEEDED:
                    test_net3 = "wrong checksum len";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(test_net3);
                    OledUpdate();
                    break;
                // if there is not enough length for the checksum then 
                // print the correct message
                case BB_ERROR_CHECKSUM_LEN_INSUFFICIENT:
                    test_net4 = "too little checksum";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(test_net4);
                    OledUpdate();
                    break;
                
                default:
                    test_net7 = "failed to parse mess.";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(test_net7);
                    OledUpdate();
                    break;
            }
        // when the state is set to start then show the field for both
        // you and your opponent as well and go into challenging state 
        // afterwards
        case BB_EVENT_START_BUTTON:
            if (player.State == AGENT_STATE_START) {
                player.place = rand() & RANDOM;
                player.mssg.param0 = NegotiationHash(player.place);
                player.mssg.type = MESSAGE_CHA;
                FieldInit(&player.own_field, &player.opponent);
                
                FieldAIPlaceAllBoats(&player.own_field);
                
                player.State = AGENT_STATE_CHALLENGING;
            } else {
                player.mssg.type = MESSAGE_NONE;
            }
            break;
        // when a challenge is received then the state is set to start
        // we then generate a random number and also initiate the has value
        // then the random number is sent to the opponent in a message
        case BB_EVENT_CHA_RECEIVED:
            if (player.State == AGENT_STATE_START) {
                player.place = rand() & RANDOM;
                player.hash = event.param0;
                player.mssg.type = MESSAGE_ACC;
                
                player.mssg.param0 = player.place;
                // fix this or change it later
                char *accepting = "ACCEPTING!";
                OledClear(OLED_COLOR_BLACK);
                OledDrawString(accepting);
                OledUpdate();
                FieldInit(&player.own_field, &player.opponent);
                FieldAIPlaceAllBoats(&player.own_field);
                player.State = AGENT_STATE_ACCEPTING;
                
            } else {
                player.mssg.type = MESSAGE_NONE;

            }
            break;
        // when we send the challenge then other steps have to be taken
        // have to flip the coin to decide who goes first and then 
        // depending on the state we decide what state to go into
        case BB_EVENT_ACC_RECEIVED:
            if (player.State == AGENT_STATE_CHALLENGING) {
                player.mssg.type = MESSAGE_REV;
                // fix this or change it later
                char *challenge = "CHALLENGING!";
                OledClear(OLED_COLOR_BLACK);
                OledDrawString(challenge);
                OledUpdate();
                AgentInit();
                player.mssg.param0 = player.place;
                NegotiationOutcome outcome = NegotiateCoinFlip(player.place, event.param0);
                // if heads it is my turn and go into an attacking mode
                if (outcome == HEADS) {
                    player_turn = FIELD_OLED_TURN_MINE;
                    player.State = AGENT_STATE_WAITING_TO_SEND;
                    // else just start defending
                } else {
                    player_turn = FIELD_OLED_TURN_THEIRS;
                    player.State = AGENT_STATE_DEFENDING;
                }
                
            } else {
                // trying to print a message for sending a challenge
                player.mssg.type = MESSAGE_NONE;
            }
            break;
        // if we accept the challenge then first you have flip the coin do decide 
        // who is going to attack first and depending on the outcome the flipped
        // coin, we go into is different state
          
        case BB_EVENT_REV_RECEIVED:
            if (player.State == AGENT_STATE_ACCEPTING) {
                NegotiationOutcome outcome = NegotiateCoinFlip(player.place, event.param0);
                // using a function from the negotiation header to detect cheating
                // from the opponent and printing a message and ending the game 
                // if it is detected
                if (NegotiationVerify(event.param0, player.hash) == FALSE) {
                    char *cheating = "cheating detected please restart!\n";
                    OledDrawString(cheating);
                    OledUpdate();
                    player.State = AGENT_STATE_END_SCREEN;
                    player.mssg.type = MESSAGE_NONE;
                    return player.mssg;
                }
                // if the outcome is tails then it is out turn and then 
                // we guess where the opponents boats are and 
                // of course go into attacking mode
                if (outcome == TAILS) {
                    player_turn = FIELD_OLED_TURN_MINE;
                    GuessData guess = FieldAIDecideGuess(&player.opponent);
                    player.mssg.type = MESSAGE_SHO;
                    player.mssg.param0 = guess.row;
                    player.mssg.param1 = guess.col;
                    player.State = AGENT_STATE_ATTACKING;
                // else or if the coin lands on tails then we go into defending 
                // and it is the opponents turn
                } else {
                    player.mssg.type = MESSAGE_NONE;
                    player_turn = FIELD_OLED_TURN_THEIRS;
                    player.State = AGENT_STATE_DEFENDING;
                }
            } else {
                player.mssg.type = MESSAGE_NONE;
            }
            break;
        // when defending when we receive the message from our opponent 
        // of where they are attacking we make the place where it lands
        // take that place and then send it back to opponent
        // if this ends the game we display the correct string
        case BB_EVENT_SHO_RECEIVED:
            if (player.State == AGENT_STATE_DEFENDING) {
                GuessData Opponent;
                Opponent.row = event.param0;
                Opponent.col = event.param1;
                // registering the attack here
                FieldRegisterEnemyAttack(&player.own_field, &Opponent);
                
                player.mssg.type = MESSAGE_RES;
                player.mssg.param0 = event.param0;
                player.mssg.param1 = event.param1;
                player.mssg.param2 = Opponent.result;
                // checking to see if the game is over
                if (FieldGetBoatStates(&player.own_field) == GAMEOVER) {
                    player.mssg.type = MESSAGE_NONE;
                    char *lose = "Alas, defeat :(\n";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(lose);
                    OledUpdate();
                    player.State = AGENT_STATE_END_SCREEN;
                    return player.mssg;
                } else {
                    player_turn = FIELD_OLED_TURN_MINE;
                    player.State = AGENT_STATE_WAITING_TO_SEND;
                }
                
            } else {
                player.mssg.type = MESSAGE_NONE;
            }
            break;
        // when we are attacking then we have to make a guess as to where 
        // out opponents boats are. If we hit one then we take that and 
        // add that data to what we know.
        // if the hit we made gives us the win then we display and do things
        // accordingly
        case BB_EVENT_RES_RECEIVED:
            if (player.State == AGENT_STATE_ATTACKING) {
                // making a guess here
                GuessData Guess;
                Guess.row = event.param0;
                Guess.col = event.param1;
                Guess.result = event.param2;
                FieldUpdateKnowledge(&player.opponent, &Guess);
                // checking to see if the hit makes us winners and printing 
                // the correct message
                if (FieldGetBoatStates(&player.opponent) == GAMEOVER) {
                    player.mssg.type = MESSAGE_NONE;
                    char *win = "Alas, victory! :)\n";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(win);
                    OledUpdate();
                    player.State = AGENT_STATE_END_SCREEN;
                    return player.mssg;
                } else {
                    player.mssg.type = MESSAGE_NONE;
                    player_turn = FIELD_OLED_TURN_THEIRS;
                    player.State = AGENT_STATE_DEFENDING;
                }
            } else {
                player.mssg.type = MESSAGE_NONE;
            }
            break;
        // if the reset button is pressed then everything has to be reset again
        // both your field and your opponents 
        // as well as displaying the correct question on the screen
        case BB_EVENT_RESET_BUTTON:
            player.mssg.type = MESSAGE_NONE;
            char *tempWelcome = "Press BTN4 for game! \nor wait for opponent!\n";
            OledClear(OLED_COLOR_BLACK);
            OledDrawString(tempWelcome);
            OledUpdate();
            AgentInit();
            return player.mssg;
            break;
        // when a message is being sent and we are attacking 
        // then we send it to the other player and get the data
        // that this message or hit causes
        case BB_EVENT_MESSAGE_SENT:
            if (player.State == AGENT_STATE_WAITING_TO_SEND) {
                counter++;
                GuessData guess = FieldAIDecideGuess(&player.opponent);
                
                player.mssg.type = MESSAGE_SHO;
                player.mssg.param0 = guess.row;
                player.mssg.param1 = guess.col;
                
                player.State = AGENT_STATE_ATTACKING;
                
            } else {
                player.mssg.type = MESSAGE_NONE;
            }
            break;
            
            player.State = AGENT_STATE_END_SCREEN;
            player.mssg.type = MESSAGE_ERROR;
            return player.mssg;
            
            break;
        // this is extra credit, decided not to do it
        case BB_EVENT_NO_EVENT:
            player.mssg.type = MESSAGE_NONE;
            break;
        case BB_EVENT_SOUTH_BUTTON:
            player.mssg.type = MESSAGE_NONE;
            break;
        case BB_EVENT_EAST_BUTTON:
            player.mssg.type = MESSAGE_NONE;
            break;
    }
    // setting the kits screen to what we get out of the switch statment
    OledClear(OLED_COLOR_BLACK);
    FieldOledDrawScreen(&player.own_field, &player.opponent, player_turn, counter);
    OledUpdate();
    return player.mssg;
}

// gets the state of the agent
AgentState AgentGetState(void) {
    return player.State;
}
// sets the state of the agent
void AgentSetState(AgentState newState) {
    player.State = newState;
}


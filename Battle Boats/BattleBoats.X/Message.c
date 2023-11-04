// Antonio Hernandez Ruiz

// standard libraries
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// user libraries
#include "BOARD.h"
#include "BattleBoats.h"
#include "Message.h"

typedef enum {
    W,
    P,
    C,
} State;

#define MONEYSIGN '$'
#define STARSIGN '*'
#define NEWLINE '\n'
#define LIMIT1 65
#define LIMIT2 57
#define LIMIT3 48
#define LIMIT4 70
#define TWO 2

static State state = W;
static char payload_dec[MESSAGE_MAX_PAYLOAD_LEN];
static char payload_en[MESSAGE_MAX_PAYLOAD_LEN];
static char temp[MESSAGE_MAX_PAYLOAD_LEN];
static char result[MESSAGE_MAX_LEN];
static uint8_t cs;
static char checkSumString[3];
static int counter = 0;

// this function just iterates through every index in the 
// string and does a bitwise XOR of all the characters
uint8_t Message_CalculateChecksum(const char* payload) {
    uint8_t x = 0;
    for(int i = 0; i < strlen(payload); i++) {
        x ^= payload[i];
    }
    return x;
}

int Message_ParseMessage(const char* payload,
        const char* checksum_string, BB_Event * message_event) {
    
    // variable to get tokens from the string
    char *token; 
    // setting all the parameters back to zero
    message_event->param0 = 0;
    message_event->param1 = 0;
    message_event->param2 = 0;
    
    strcpy(payload_en, payload);
    // checking the length of the checksum string to make sure it 
    // is of length 2
    // if not an error is raised
    if (strlen(checksum_string) != 2) {
        message_event->type = BB_EVENT_ERROR;
        return STANDARD_ERROR;
    }
    // setting the values in the checksum string
    // to be in base 16
    uint8_t hexa = strtoul(checksum_string, NULL, 16);
    // the checksum value of payload has to be the same as
    // the values we just got if not we raise an
    // error
    if (Message_CalculateChecksum(payload) != hexa) {
        message_event->type = BB_EVENT_ERROR;
        return STANDARD_ERROR;
    }
    // getting what is in the string in between all the commas
    token = strtok(payload_en, ",");
    // setting up a counter to get the amount of characters 
    // that are usually in the different type of 
    // messages
    // if the message is not any of the ones listed below we raise
    // an error
    int j;
    if (strcmp(token, "CHA") == 0) {
        j = 1;
        message_event->type = BB_EVENT_CHA_RECEIVED;
    } else if (strcmp(token, "ACC") == 0) {
        j = 1;
        message_event->type = BB_EVENT_ACC_RECEIVED;
    } else if (strcmp(token, "SHO") == 0) {
        j = 2;
        message_event->type = BB_EVENT_SHO_RECEIVED;
    } else if (strcmp(token, "REV") == 0) {
        j = 1;
        message_event->type = BB_EVENT_REV_RECEIVED;
    } else if (strcmp(token, "RES") == 0) {
        j = 3;
        message_event->type = BB_EVENT_RES_RECEIVED;
    } else {
        message_event->type = BB_EVENT_ERROR;
        return STANDARD_ERROR;
    }
    // checking to see if the amount of characters is correct
    // for the type of message we are parsing
    int i;
    for (i = 0; i < j; i++) {
        token = strtok(NULL, ",");
        
        if (token == NULL) {
            message_event->type = BB_EVENT_ERROR;
            return STANDARD_ERROR;
        }
        // changing the numbers that come in the messages to integers
        // then putting them into the corresponding parameter
        uint16_t x = atoi(token);
        if (i == 0) {
            message_event->param0 = x;
        } else if (i == 1) {
            message_event->param1 = x;
        } else if (i == 2) {
            message_event->param2 = x;
        }
    }
    // now we check the length of the string again
    // after checking all the characters that should be their
    // the next one should be null
    // if it is not we raise an error
    token = strtok(NULL, ",");
    if (token) {
        message_event->type = BB_EVENT_ERROR;
        return STANDARD_ERROR;
    }
    
    return SUCCESS;
    
    
}

int Message_Encode(char *message_string, Message message_to_encode) {
    // a big switch statement that checks the different types of messages
    // when the type of message is known then is is added to a template
    // afterwards it is put inside of another variable with the checksum
    // added to it and then put into the final item we are supposed to return
    // that being the message_string
    switch (message_to_encode.type) {
        case MESSAGE_REV:
            sprintf(temp, PAYLOAD_TEMPLATE_REV, message_to_encode.param0);
            cs = Message_CalculateChecksum(temp);
            sprintf(result, MESSAGE_TEMPLATE, temp, cs);
            strcpy(message_string, result);
            break;
        case MESSAGE_ACC:
            sprintf(temp, PAYLOAD_TEMPLATE_ACC, message_to_encode.param0);
            cs = Message_CalculateChecksum(temp);
            sprintf(result, MESSAGE_TEMPLATE, temp, cs);
            strcpy(message_string, result);
            break;
        case MESSAGE_CHA:
            sprintf(temp, PAYLOAD_TEMPLATE_CHA, message_to_encode.param0);
            cs = Message_CalculateChecksum(temp);
            sprintf(result, MESSAGE_TEMPLATE, temp, cs);
            strcpy(message_string, result);
            break;
        case MESSAGE_SHO:
            sprintf(temp, PAYLOAD_TEMPLATE_SHO, message_to_encode.param0, message_to_encode.param1);
            cs = Message_CalculateChecksum(temp);
            sprintf(result, MESSAGE_TEMPLATE, temp, cs);
            strcpy(message_string, result);
            break;
        case MESSAGE_RES:
            sprintf(temp, PAYLOAD_TEMPLATE_RES, message_to_encode.param0, message_to_encode.param1, message_to_encode.param2);
            cs = Message_CalculateChecksum(temp);
            sprintf(result, MESSAGE_TEMPLATE, temp, cs);
            strcpy(message_string, result);
            break;
        // when no message is passed we return zero and then break
        // the same is done for a message error except we just
        // break
        case MESSAGE_NONE:
            return 0;
            break;
        case MESSAGE_ERROR:
            break;
    }
    // the string length is returned
    return strlen(message_string);
}



int Message_Decode(unsigned char char_in, BB_Event * decoded_message_event) {
    // another switch statement that checks for the different message types that
    // can be passed in
    switch (state) {
        // first we check if the character passed in is a '$'
        // if it isn't or it is not in the correct position
        // then we return an error
        case W:
            if (char_in == MONEYSIGN) {
                decoded_message_event->type = BB_EVENT_NO_EVENT;
                state = P;
            } else {
                decoded_message_event->type = BB_EVENT_ERROR;
                decoded_message_event->param0 = BB_ERROR_INVALID_MESSAGE_TYPE;
                return STANDARD_ERROR;
            }
            break;
        case P:
            // for the second state we check for a '*' and try and make sure 
            // that it is in the right position
            if (counter > MESSAGE_MAX_PAYLOAD_LEN) {
                counter = 0;
                decoded_message_event->type = BB_EVENT_ERROR;
                decoded_message_event->param0 = BB_ERROR_PAYLOAD_LEN_EXCEEDED;
                state = W;
                
                return STANDARD_ERROR;
            // if the character passed in is another symbol then we
            // raise an error
            } else if (char_in == NEWLINE || char_in == MONEYSIGN) {
                counter = 0;
                decoded_message_event->type = BB_EVENT_ERROR;
                decoded_message_event->param0 = BB_ERROR_INVALID_MESSAGE_TYPE;
                state = W;
                
                return STANDARD_ERROR;
                // if the character is a '*' we don't do anything
                // just set the correct state and reset the counter
                // and end with a null
            } else if (char_in == STARSIGN) {
                decoded_message_event->type = BB_EVENT_NO_EVENT;
                payload_dec[counter] = '\0';
                state = C;
                counter = 0;
            } else {
                decoded_message_event->type = BB_EVENT_NO_EVENT;
                payload_dec[counter] = char_in;
                counter++;
            }
            break;
        case C:
            // for this third state we do the same thing as the last one
            // check to see if the string is the correct length first
            if (counter > TWO) {
                counter = 0;
                decoded_message_event->type = BB_EVENT_ERROR;
                decoded_message_event->param0 = BB_ERROR_CHECKSUM_LEN_EXCEEDED;
                state = W;
                return STANDARD_ERROR;
               // if it is the correct character we check the length again
               // to make sure it is correct
            } else if (char_in == NEWLINE) {
                if (counter < TWO) {
                    counter = 0;
                    decoded_message_event->type = BB_EVENT_ERROR;
                    decoded_message_event->param0 = BB_ERROR_CHECKSUM_LEN_INSUFFICIENT;
                    state = W;
                    return STANDARD_ERROR;
                // if everything is good we set the counter to zero and add a
                  // null character at the end
                } else {
                    checkSumString[counter] = '\0';
                    counter = 0;
                    // checking to see if the message that we received can be 
                    // parsed
                    int x = Message_ParseMessage(payload_dec, checkSumString, decoded_message_event);
                    if (x == STANDARD_ERROR) {
                        decoded_message_event->type = BB_EVENT_ERROR;
                        decoded_message_event->param0 = BB_ERROR_MESSAGE_PARSE_FAILURE;
                        state = W;
                        return STANDARD_ERROR;
                    }
                    state = W;
                }
                // if the character is not a newline we check to see if it is
                // within the correct limits of characters
                // that we can decode
            } else if ((char_in < LIMIT1 && char_in > LIMIT2) || char_in < LIMIT3 || char_in > LIMIT4) {
                counter = 0;
                decoded_message_event->type = BB_EVENT_ERROR;
                decoded_message_event->param0 = BB_ERROR_BAD_CHECKSUM;
                state = W;
                return STANDARD_ERROR;
            } else {
                decoded_message_event->type = BB_EVENT_NO_EVENT;
                checkSumString[counter] = char_in;
                counter++;
            }
            break;
    }
    
    return SUCCESS;
}
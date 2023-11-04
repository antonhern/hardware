// Antonio Hernandez Ruiz

// standard libraries
#include <stdio.h>
#include <stdlib.h>
// user libraries
#include "Agent.h"
#include "BattleBoats.h"
#include "BOARD.h"
int main() { 
    printf("\nTest for AgentSetState() and AgentGetState():\n");    
    //counter to get the amount of tests/states that the agent can 
    // be set to successfully
    int counter = 0;
    // testing both AgentSetState and AgentGetState at the same time
    // by first setting the state and testing if the state that the next
    // function gets is the same
    // setting and getting multiple states to test if all of them work
    AgentSetState(AGENT_STATE_START);
    if(AgentGetState() == AGENT_STATE_START){
        counter++;
    }else{
        printf("\nfirst test failed");
    }
    AgentSetState(AGENT_STATE_CHALLENGING);
    if(AgentGetState() == AGENT_STATE_CHALLENGING){
        counter++;
    }else{
        printf("\nsecond test failed");
    }
    AgentSetState(AGENT_STATE_ACCEPTING);
    if(AgentGetState() == AGENT_STATE_ACCEPTING){
        counter++;
    }else{
        printf("\nthird test failed");
    }
    AgentSetState(AGENT_STATE_ATTACKING);
    if(AgentGetState() == AGENT_STATE_ATTACKING){
        counter++;
    }else{
        printf("\nfourth test failed");
    }
    AgentSetState(AGENT_STATE_DEFENDING);
    if(AgentGetState() == AGENT_STATE_DEFENDING){
        counter++;
    }else{
        printf("\nfifth test failed");
    }
    AgentSetState(AGENT_STATE_WAITING_TO_SEND);
    if(AgentGetState() == AGENT_STATE_WAITING_TO_SEND){
        counter++;
    }else{
        printf("\nsixth test failed");
    }
    AgentSetState(AGENT_STATE_END_SCREEN);
    if(AgentGetState() == AGENT_STATE_END_SCREEN){
        counter++;  
    } else{
        printf("\nseventh test failed");
    }
    if(counter == 7){
        printf("Tests Passed\n");
    } 
    // using the same kind of test for agent init just initializing the state 
    // and then testing if that is the correct one
    printf("Test for AgentInit():\n");
    AgentInit();
    if(AgentGetState() == AGENT_STATE_START){
        printf("Test Passed\n");
    }else{
        printf("\nAgentInit() test failed");
    }    
    
    // using an event and setting parameters for that event to test agent run
    // this would be the same as setting the state as a challenge 
    printf("Testing AgentRun:\n");
    counter = 0;
    BB_Event event;
    event.type = BB_EVENT_START_BUTTON;
    event.param0 = 0;
    event.param1 = 0;
    event.param2 = 0;
    AgentRun(event);    
    if(AgentGetState() == AGENT_STATE_CHALLENGING){
        counter++;   
    }else{
        printf("\nAgentRun() test failed");
    }    
    if(counter == 1){
        printf("\nTest Passed\n");  
    }   
    
    while(1);    
}
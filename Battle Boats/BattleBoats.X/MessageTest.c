// Antonio Hernandez Ruiz

// standard libraries

#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
// user libraries
#include "BOARD.h"
#include "Message.h"
#include "OledDriver.h"
#include "Oled.h"
#include "Ascii.h"
#include "xc.h"

#define TEST1 0x57
#define TEST2 0x58
#define ZERO 0
#define ONE 1

static uint8_t checksum;
static char test_message[MESSAGE_MAX_LEN];
static int output;
static int counter = 0;
static BB_Event Event;
static Message mssg;
int main() {
    // testing message encode based on the messages that I saw the python scripts
    // and the message templates that are provided in the header file 
    // for message
    // testing to see if the message produced is the correct one using strcmp
    printf("Message_Encode() Test:\n\n");
    
    mssg.type = MESSAGE_CHA;
    mssg.param0 = 1;
    Message_Encode(test_message, mssg);
    if (strcmp(test_message, "$CHA,1*57\n") == ZERO) {
        printf("\tPassed test1!\n");
    } else {
        printf("\tFailed test1!\n");
        // adding to the counter so in the end I can tell whether all the tests 
        // passed or some of them failed
        counter = ONE;
    }
    // using another of the templates from the head that I know what the encoded
    // message looks like
    mssg.type = MESSAGE_SHO;
    mssg.param0 = 1;
    mssg.param1 = 2;
    Message_Encode(test_message, mssg);
    if (strcmp(test_message, "$SHO,1,2*57\n") == ZERO) {
        printf("\tPassed test2!\n");
    } else {
        printf("\tFailed test2!\n");
        counter = ONE;
    }
    // same thing here just have to set the other parameters as well.
    mssg.type = MESSAGE_RES;
    mssg.param0 = 1;
    mssg.param1 = 2;
    mssg.param2 = 3;
    Message_Encode(test_message, mssg);
    if (strcmp(test_message, "$RES,1,2,3*58\n") == ZERO) {
        printf("\tPassed test3!\n\n");
    } else {
        printf("\tFailed test3!\n\n");
        counter = ONE;
    }
    
    // for this test I got the checksum from some input and then tested it 
    // with its value with a bitwise XOR
    // did that for 3 different inputs
    printf("Message_CalculateChecksum() Test:\n\n");
    
    // the bitwise XOR values are above as #defines
    char temp1[] = "CHA,1";
    checksum = Message_CalculateChecksum(temp1);
    if (checksum == TEST1) {
        printf("\tPassed test1!\n");
    } else {
        printf("\tFailed test1!!\n");
        counter = ONE;
    }
    
    char temp2[] = "SHO,1,2";
    checksum = Message_CalculateChecksum(temp2);
    if (checksum == TEST1) {
        printf("\tPassed test2!\n");
    } else {
        printf("\tFailed test2!\n");
        counter = ONE;
    }
    
    char temp3[] = "RES,1,2,3";
    checksum = Message_CalculateChecksum(temp3);
    if (checksum == TEST2) {
        printf("\tPassed test3!\n\n");
    } else {
        printf("\tFailed test3! \n\n");
        counter = ONE;
    }
    
    
    printf("Testing Message_Decode():\n\n");
    // testing message decode by decoding messages that I know should produce 
    // certain events, the tests are done by pushing each 
    // character one be on until all of them have been pushed 
    // and then checking if the correct event is set at the end
    int i = ZERO;
    // checking the event for challenge
    char temp8[] = "$CHA,1*57\n";
    for (i = ZERO; i < strlen(temp8); i++) {
        Message_Decode(temp8[i], &Event);
    } 
    if (Event.type == BB_EVENT_CHA_RECEIVED && Event.param0 == 1) {
        printf("\tPassed test1!\n");
    } else {
        printf("\tFailed test! %d\n", Event.param0);
        counter = ONE;
    }
    // checking the event for shot
    char temp9[] = "$SHO,1,2*57\n";
    for (i = ZERO; i < strlen(temp9); i++) {
        Message_Decode(temp9[i], &Event);
    } 
    if (Event.type == BB_EVENT_SHO_RECEIVED && Event.param0 == 1 && Event.param1 == 2) {
        printf("\tPassed test2!\n");
    } else {
        printf("\tFailed test2! %d\n", Event.param0);
        counter = ONE;
    }
    // checking another event here as well
    char temp10[] = "$RES,1,2,3*58\n";
    for (i = ZERO; i < strlen(temp10); i++) {
        Message_Decode(temp10[i], &Event);
    } 
    if (Event.type == BB_EVENT_RES_RECEIVED && Event.param0 == 1 && Event.param1 == 2 && Event.param2 == 3) {
        printf("\tPassed test3!\n");
    } else {
        printf("\tFailed test3! %d\n", Event.param0);
        counter = ONE;
    }
    // checking to see if errors are raised for trying to decode a message
    // that does not follow the correct format
    char temp11[] = "RES,1,5,3*58\n";
    for (i = ZERO; i < strlen(temp11); i++) {
        Message_Decode(temp11[i], &Event);
    } 
    if (Event.type == BB_EVENT_ERROR) {
        printf("\tPassed test4!\n\n");
    } else {
        printf("\tFailed test4!\n\n");
        counter = ONE;
    }
     printf("Message_ParseMessage() Test:\n\n");
    // checking to see if the event returned is the correct one
    // almost the same as the encode message tests but checks for
    // some different things
     // first checking the challenge event
    char temp4[] = "CHA,1";
    char compare1[] = "57";
    output = Message_ParseMessage(temp4, compare1, &Event);
    if (output && Event.type == BB_EVENT_CHA_RECEIVED && Event.param0 == 1 ) {
        printf("\tPassed test1!\n");
    } else {
        printf("\tFailed test1!\n");
        counter = ONE;
    }
    // checking the shot event 
    char temp5[] = "SHO,1,2";
    char compare2[] = "57";
    output = Message_ParseMessage(temp5, compare2, &Event);
    if (output && Event.type == BB_EVENT_SHO_RECEIVED && Event.param0 == 1 && Event.param1 == 2) {
        printf("\tPassed test2!\n");
    } else {
        printf("\tFailed test2!\n");
        counter = ONE;
    }
    // checking another event
    char temp6[] = "RES,1,2,3";
    char compare3[] = "58";
    output = Message_ParseMessage(temp6, compare3, &Event);
    if (output && Event.type == BB_EVENT_RES_RECEIVED && Event.param0 == 1 && Event.param1 == 2
            && Event.param2 == 3) {
        printf("\tPassed test3!\n");
    } else {
        printf("\tFailed test3!\n");
        counter = ONE;
    }
    // checking for wrong input here to see if errors are raised
    char temp7[] = "1";
    char compare4[] = "31";
    output = Message_ParseMessage(temp7, compare4, &Event);
    if (!output && Event.type == BB_EVENT_ERROR) {
        printf("\tPassed test4!\n");
    } else {
        printf("\tFailed test4!\n");
        counter = ONE;
    }
    if (!counter) {
        printf("\nALL TESTS PASSED!\n");
    } else {
        printf("\nSOME TESTS FAILED!\n");
    }
    
    while (1);
    
}
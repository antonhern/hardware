//Antonio Hernandez Ruiz

// **** Include libraries here ****
// Standard libraries
#include <stdio.h>

//CMPE13 Support Library
#include "BOARD.h"
#include "Buttons.h"

// Microchip libraries
#include <xc.h>
#include <sys/attribs.h>

// User libraries

// **** Set macros and preprocessor directives ****

// **** Declare any datatypes here ****
static uint8_t timer;
// **** Define global, module-level, or external variables here ****

// **** Declare function prototypes ****

int main(void)
{
    BOARD_Init();

    // Configure Timer 1 using PBCLK as input. This default period will make the LEDs blink at a
    // pretty reasonable rate to start.
    T1CON = 0; // everything should be off
    T1CONbits.TCKPS = 2; // 1:64 prescaler
    PR1 = 0xFFFF; // interrupt at max interval
    T1CONbits.ON = 1; // turn the timer on

    // Set up the timer interrupt with a priority of 4.
    IFS0bits.T1IF = 0; //clear the interrupt flag before configuring
    IPC1bits.T1IP = 4; // priority of  4
    IPC1bits.T1IS = 0; // subpriority of 0 arbitrarily 
    IEC0bits.T1IE = 1; // turn the interrupt on

    /***************************************************************************************************
     * Your code goes in between this comment and the following one with asterisks.
     **************************************************************************************************/
    printf("Welcome to ahern336's lab6 part4 (ButtonsTest).  Compiled on %s %s.\n", __TIME__, __DATE__);

    
    printf("Please press some buttons!\n");
    //initializing the buttons
    ButtonsInit();
    while(1){
        //printing the lines that are shown in the example
        char *line1 = "----";
        char *line2 = "----";
        char *line3 = "----";
        char *line4 = "----";
        //depending on what button is up we print that line as up 
        //same thing for down 
        if(timer != BUTTON_EVENT_NONE){
            if(timer & BUTTON_EVENT_1UP){
                line1 = "UP";
            }
            if(timer & BUTTON_EVENT_2UP){
                line2 = "UP";
            }
            if(timer & BUTTON_EVENT_3UP){
                line3 = "UP";
            }
            if(timer & BUTTON_EVENT_4UP){
                line4 = "UP";
            }
            if(timer & BUTTON_EVENT_1DOWN){
                line1 = "DOWN";
            }
            if(timer & BUTTON_EVENT_2DOWN){
                line2 = "DOWN";
            }
            if(timer & BUTTON_EVENT_3DOWN){
                line3 = "DOWN";
            }
            if(timer & BUTTON_EVENT_4DOWN){
                line4 = "DOWN";
            }
            timer = BUTTON_EVENT_NONE;
            //printing the result
            printf("Event: 4:%s 3:%s 2:%s 1:%s\n", line4, line3, line2, line1);
        }
    }
    
    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/

    while (1);
}

/**
 * This is the interrupt for the Timer1 peripheral. It should check for button events and stores them in a
 * module-level variable.
 * 
 * You should not modify this function for ButtonsTest.c or bounce_buttons.c!
 */
void __ISR(_TIMER_1_VECTOR, ipl4auto) Timer1Handler(void)
{
    // Clear the interrupt flag.
    IFS0bits.T1IF = 0;

    /***************************************************************************************************
     * Your code goes in between this comment and the following one with asterisks.
     **************************************************************************************************/
    timer = ButtonsCheckEvents();

    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/

}
//Antonio Hernandez Ruiz

// **** Include libraries here ****
// old bounce
// Standard libraries
#include <stdio.h>
#include "Leds_Lab06.h"					


//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <sys/attribs.h>

// **** Declare any datatypes here ****
struct Timer{
    uint8_t event;
    int16_t timeRemaining;
};

// **** Define global, module-level, or external variables here ****
static struct Timer Timer1;
#define LEFT 0
#define RIGHT 1

// **** Declare function prototypes ****


int main(void)
{
    BOARD_Init();

    // Configure Timer 1 using PBCLK as input. This default period will make the LEDs blink at a
    // pretty reasonable rate to start.
    T1CON = 0; // everything should be off
    T1CONbits.TCKPS = 1; // 1:8 prescaler
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
    printf("Welcome to ahern336's lab6 part2 (bounce_switch).  Compiled on %s %s.\n",__TIME__,__DATE__);
    //start with a false event
    Timer1.event = FALSE;
    
    //you don't want to initialize any values into state so give it -1.
    int state = -1;
    //the limits to where the LEDS can 'reach'.
    char leftLimit = 0x80;
    char rightLimit = 0x01;
    
    //The first LED that should be lit up.
    char start = 0x01;
    
    LEDS_INIT();
							 
	while(1){
        //poll timer events and react if any occur
        //checking for timer events
        if(Timer1.event){
            //start the led at the first one
            LEDS_SET(start);
            //if the LED is all the way on the left then we want it to move to the right.
            //same thing with the right LED
            if(start == leftLimit){
                state = RIGHT;
            }else if(start == rightLimit){
                state = LEFT;
            }
            //move the LED right by one here.
            if(state == LEFT){
                start <<=1;
            }else{
                //making it so the LEDs don't say lit up as they go through 
                if(start == leftLimit){
                    start = (start >> 1) ^ start;
                }else{
                    start >>=1;
                }
            }
            //end the if statement here 
            Timer1.event = FALSE;
        }
    }			


    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/

}


/**
 * This is the interrupt for the Timer1 peripheral. It will trigger at the frequency of the peripheral
 * clock, divided by the timer 1 prescaler and the interrupt interval.
 * 
 * It should not be called, and should communicate with main code only by using module-level variables.
 */
void __ISR(_TIMER_1_VECTOR, ipl4auto) Timer1Handler(void)
{
    // Clear the interrupt flag.
    IFS0bits.T1IF = 0;

    /***************************************************************************************************
     * Your code goes in between this comment and the following one with asterisks.
     **************************************************************************************************/
    Timer1.timeRemaining -=1; //decrement the time remaining or speed
    int faster = 3;//make things faster 
    if(Timer1.timeRemaining <= 0){
        Timer1.event = TRUE;
        
        //count how many switches are flipped and will affect the speed.
        uint8_t flippedSwitch = 0x00;
        //getting all the different states of the switches.
        uint8_t switchStates = SWITCH_STATES();
        //accounting for any flipped switches.
        if(switchStates & SWITCH_STATE_SW1){
            flippedSwitch |= SWITCH_STATE_SW1;
        }
        if(switchStates & SWITCH_STATE_SW2){
            flippedSwitch |= SWITCH_STATE_SW2;
        }
        if(switchStates & SWITCH_STATE_SW3){
            flippedSwitch |= SWITCH_STATE_SW3;
        }
        if(switchStates & SWITCH_STATE_SW4){
            flippedSwitch |= SWITCH_STATE_SW4;
        }
        //adding this so the LED moves faster the difference was not noticeable before
        flippedSwitch = flippedSwitch * faster;
        Timer1.timeRemaining = flippedSwitch;
    }
    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/									
	 
}
//Antonio Hernandez Ruiz

// Standard libraries
#include <stdio.h>
#include <GenericTypeDefs.h>

//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <sys/attribs.h>

// User libraries
#include "Leds_Lab06.h"


// **** Declare any datatypes here ****

struct Timer {
    uint8_t event;
    int16_t timeRemaining;
};

// **** Define global, module-level, or external variables here ****
static struct Timer TimerA;
static struct Timer TimerB;
static struct Timer TimerC;
// **** Declare function prototypes ****
#define TIME_A 76
#define TIME_B 114
#define TIME_C 190

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
    printf("Welcome to ahern336's lab6 part1 (timers).  Compiled on %s %s.\n", __TIME__, __DATE__);
    
    // initialize the led to get ready to light up
    LEDS_INIT();
    
    TimerA.event = FALSE;
    TimerB.event = FALSE;
    TimerC.event = FALSE;
    
    TimerA.timeRemaining = TIME_A;
    TimerB.timeRemaining = TIME_B;
    TimerC.timeRemaining = TIME_C;
    //value to change and modify the LEDS
    char value = LEDS_GET();
    //while loop to check for any any events
    while (1) {
    //modify the value or the LATE of the LED for A B and C  
        if (TimerA.event == TRUE) {
            printf("A ");
            TimerA.event = FALSE;
            value ^= 0x01;
        }
        if (TimerB.event == TRUE) {
            printf("B ");
            TimerB.event = FALSE;
            value ^= 0x02;
        }
        if (TimerC.event == TRUE) {
            printf("C ");
            TimerC.event = FALSE;
            value ^= 0x04;
        }
        //return any events continously throughout the loop
        LEDS_SET(value);
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
    
 
    TimerA.timeRemaining--;
    if (!TimerA.timeRemaining) {
        TimerA.event = TRUE;
        TimerA.timeRemaining = TIME_A;
    }
    
    TimerB.timeRemaining--;
    if (!TimerB.timeRemaining) {
        TimerB.event = TRUE;
        TimerB.timeRemaining = TIME_B;
    }
    
    TimerC.timeRemaining--;
    if (!TimerC.timeRemaining) {
        TimerC.event = TRUE;
        TimerC.timeRemaining = TIME_C;
    }
    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/

}
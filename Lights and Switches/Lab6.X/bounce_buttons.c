//Antonio Hernandez Ruiz

// **** Include libraries here ****
// Standard libraries
#include <stdio.h>

//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <sys/attribs.h>

// User libraries
#include "Buttons.h"
#include "Leds_Lab06.h"
// **** Set macros and preprocessor directives ****
//all the values for the LEDS

#define LED0  0x00
#define LED1  0x01
#define LED2  0x02
#define LED3  0x04
#define LED4  0x08
#define LED5  0x10
#define LED6  0x20
#define LED7  0x40
#define LED8  0x80
            
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
    printf("Welcome to ahern336's lab6 part5 (bounce_buttons).  Compiled on %s %s.\n", __TIME__, __DATE__);
    LEDS_INIT();
    ButtonsInit();
    while(1){
        if(timer != BUTTON_EVENT_NONE){
            //getting the position of the LED at the moment
            char LED = LEDS_GET();
            // getting the state of the switches if they are up or down
            uint8_t switches = SWITCH_STATES();
            //correlating the switch movement with what i need the buttons to do 
            // then setting how the LEDs light up according if switch one is up or down
            // using xor do define the led
            if((switches & SWITCH_STATE_SW4)&& (timer & BUTTON_EVENT_4UP)){
                LED = LED ^ LED7 ^ LED8;
            }else if(!(switches & SWITCH_STATE_SW4) && timer & BUTTON_EVENT_4DOWN){
                LED = LED ^ LED7 ^ LED8;
            }
            if((switches & SWITCH_STATE_SW3) && (timer & BUTTON_EVENT_3UP)){
                LED = LED ^ LED5 ^ LED6;
            }else if(!(switches & SWITCH_STATE_SW3) && timer & BUTTON_EVENT_3DOWN){
                LED = LED ^ LED5 ^ LED6;
            }
            if((switches & SWITCH_STATE_SW2) && (timer & BUTTON_EVENT_2UP)){
                LED = LED ^ LED3 ^ LED4;
            }else if(!(switches & SWITCH_STATE_SW2) && timer & BUTTON_EVENT_2DOWN){
                LED = LED ^ LED3 ^ LED4;
            }
            if((switches & SWITCH_STATE_SW1)&& (timer & BUTTON_EVENT_1UP)){
                LED = LED ^ LED1 ^ LED2;
            }else if(!(switches & SWITCH_STATE_SW1) && timer & BUTTON_EVENT_1DOWN){
                LED = LED ^ LED1 ^ LED2;
            }
            //Lighting up the LED continously
            LEDS_SET(LED);
            timer = BUTTON_EVENT_NONE;
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
    //setting the value for the timer
    timer = ButtonsCheckEvents();
    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/

}
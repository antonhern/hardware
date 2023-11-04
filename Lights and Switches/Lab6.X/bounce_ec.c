//Antonio Hernandez Ruiz

// **** Include libraries here ****
// Standard libraries
#include <stdio.h>
#include <string.h>
#include <math.h>
//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <sys/attribs.h>

// User libraries
#include "Leds_Lab06.h"
#include "Oled.h"
#include "Buttons.h"
#include "OledDriver.h"
// **** Set macros and preprocessor directives ****

// **** Declare any datatypes here ****
struct AdcResult{
    uint8_t event;
    uint16_t voltage;
};
struct Timer{
    uint8_t event;
    int16_t timeRemaining;
};
// **** Define global, module-level, or external variables here ****
static uint8_t tiempo;
static struct AdcResult AdcResult1;
static struct Timer Timer1;
uint16_t average;

#define LEFT 0
#define RIGHT 1
#define windowSize 5
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


    // Enable interrupts for the ADC
    IPC6bits.AD1IP = 2;
    IPC6bits.AD1IS = 0;
    IEC1bits.AD1IE = 1;

    // Set B2 to an input so AN2 can be used by the ADC.
    TRISBbits.TRISB2 = 1;

    // Configure and start the ADC
    AD1CHSbits.CH0SA = 2; // add B2 to the mux
    AD1PCFGbits.PCFG2 = 0; // add b2 to the ADC
    AD1CSSLbits.CSSL2 = 1; // and add b2 to the scanner

    AD1CON1 = 0; // start with 0
    AD1CON1bits.SSRC = 0b111; // auto conversion
    AD1CON1bits.ASAM = 1; // auto sample

    AD1CON2 = 0; // start with 0
    AD1CON2bits.SMPI = 7; // one interrupt per 8 samples

    AD1CON3 = 0; // start with 0
    AD1CON3bits.SAMC = 29; // long sample time
    AD1CON3bits.ADCS = 50; // long conversion time

    AD1CON1bits.ADON = 1; // and finally turn it on

    /***************************************************************************************************
     * Your code goes in between this comment and the following one with asterisks.
     **************************************************************************************************/
  printf("Welcome to ahern336's lab6 part6 (Extra Credit).  Compiled on %s %s.\n",__TIME__,__DATE__);
  OledInit();
  AdcResult1.event = TRUE;
  char display[OLED_CHARS_PER_LINE];
  
  ButtonsInit();
  LEDS_INIT();
  Timer1.event = FALSE;
  int stop = 0;
  int state = -1;
  char rightLimit = 0x01;
  char leftLimit = 0x80;
  char start = 0x01;
  // Not sure if I did the correct extra credit because the lab doc said to do it this way and the demo said to do 
  // something completely different I hope I don't get any less credit
  while(1){
      if(AdcResult1.event){
          int percent = AdcResult1.voltage / 1023.0 * 100;
          sprintf(display, "Potentiometer value:\n  %d\n  %d%%", AdcResult1.voltage, percent);
          OledClear(OLED_COLOR_BLACK);
          OledDrawString(display);
          OledUpdate();
          AdcResult1.event = FALSE;
      }
      //checking for any button events and switch states if the first switch is up 
      // the led gets stopped by a push down and with switch 1 up the light
      //gets stopped by a button up
      //if a button is pressed I activate a variable called stop
      if(tiempo != BUTTON_EVENT_NONE){
          uint8_t switches = SWITCH_STATES();
          
          if((switches & SWITCH_STATE_SW1) && (tiempo & BUTTON_EVENT_4UP)){
              if(stop){
                  stop = 0;
              }else{
                  stop = 1;
              }
          }else if(!(switches & SWITCH_STATE_SW1) && (tiempo & BUTTON_EVENT_4DOWN)){
              if(stop){
                  stop = 0;
              }else{
                  stop = 1;
              }             
          }
          else if((switches & SWITCH_STATE_SW1) && (tiempo & BUTTON_EVENT_3UP)){
              if(stop){
                  stop = 0;
              }else{
                  stop = 1;
              }
          }else if(!(switches & SWITCH_STATE_SW1) && (tiempo & BUTTON_EVENT_3DOWN)){
              if(stop){
                  stop = 0;
              }else{
                  stop = 1;
              }
          }
          else if((switches & SWITCH_STATE_SW1) && (tiempo & BUTTON_EVENT_2UP)){
              if(stop){
                  stop = 0;
              }else{
                  stop = 1;
              }            
          }else if(!(switches & SWITCH_STATE_SW1) && (tiempo & BUTTON_EVENT_2DOWN)){
              if(stop){
                stop = 0;  
              }else{
                  stop = 1;
              }
          }
          else if((switches & SWITCH_STATE_SW1) && (tiempo & BUTTON_EVENT_1UP)){
              if(stop){
                  stop = 0;
              }else{
                  stop = 1;
              }
          }else if(!(switches & SWITCH_STATE_SW1) && (tiempo & BUTTON_EVENT_1DOWN)){
              if(stop){
                  stop = 0;              
              }else{
                  stop = 1;
              }
          }                  
       tiempo = BUTTON_EVENT_NONE;
      }
      //if stop is activated by a previous button press then the LED stops moving
      //because this conditional does not get set
      if(Timer1.event && stop == 0){
          LEDS_SET(start);
          if(start == leftLimit){
              state = RIGHT;
          }else if(start == rightLimit){
              state = LEFT;
          }
          if(state == LEFT){
              start <<=1;
          }else{
              if(start == leftLimit){
                  start = (start >> 1) ^ start;
              }else{
                  start >>=1;
              }
          }
          Timer1.event = FALSE;
      }
       
  }

    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/

    while (1);
}

/**
 * This is the interrupt for the Timer1 peripheral.
 */
void __ISR(_TIMER_1_VECTOR, IPL4AUTO) Timer1Handler(void)
{
    // Clear the interrupt flag.
    IFS0bits.T1IF = 0;
    //making the value of time remaining equal to the change in voltage
    Timer1.timeRemaining -= 1;
    if(Timer1.timeRemaining <= 0){
        
        Timer1.event = TRUE;
    
        Timer1.timeRemaining = 100 - ((AdcResult1.voltage / 1023.0)*100);
    }
    tiempo = ButtonsCheckEvents();
}

/**
 * This is the ISR for the ADC1 peripheral.
 */
void __ISR(_ADC_VECTOR, IPL2AUTO) AdcHandler(void)
{
    // Clear the interrupt flag.
    IFS1bits.AD1IF = 0;
    //same code from adc
    average = ADC1BUF0 + ADC1BUF1 + ADC1BUF2 + ADC1BUF3 + ADC1BUF4 + ADC1BUF5 + ADC1BUF6 + ADC1BUF7;
    average = average / 8;
    //checking if the average is within the correct scope
    if(abs(average - AdcResult1.voltage) >= windowSize || abs(AdcResult1.voltage - average) >= windowSize){
        AdcResult1.event = TRUE;
        //accounting for edge cases(just in case)
    if(average + windowSize >= 1023){
        AdcResult1.voltage = 1023;
    }else if(average - windowSize <= 0){
        AdcResult1.voltage = 0;
    }else{
        AdcResult1.voltage = average;
    }
}

}
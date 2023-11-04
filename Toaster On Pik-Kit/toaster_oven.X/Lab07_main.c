// Antonio Hernandez Ruiz

// **** Include libraries here ****
// Standard libraries
#include <stdio.h>
#include <string.h>
//CSE13E Support Library
#include "BOARD.h"
#include "Oled.h"
#include "Leds.h"
#include "Ascii.h"
#include "Buttons.h"
#include "OledDriver.h"
#include "Adc.h"

// Microchip libraries
#include <xc.h>
#include <sys/attribs.h>



// **** Set any macros or preprocessor directives here ****
// Set a macro for resetting the timer, makes the code a little clearer.
#define TIMER_2HZ_RESET() (TMR1 = 0)


// **** Set any local typedefs here ****

typedef enum {
    SETUP, SELECTOR_CHANGE_PENDING, COOKING, RESET_PENDING, EC
} OvenState;

typedef struct {
    OvenState state;
    uint16_t cookingTleft; // stores the time change when cooking
    uint16_t initialCtime; // stores the time that we are cooking
    uint16_t temp; // stores the temperature
    uint16_t buttonPtime; // stores the amount of time the button is pressed
    uint8_t modes; // stores the cooking mode the toaster is in
    //add more members to this struct
} OvenovenData;

typedef enum {
    BAKE, TOAST, BROIL
} Cstates;


// **** Declare any ovenDatatypes here ****
static OvenovenData data;
static uint8_t temp_move = FALSE; //variable to check if the temperature is being modified
static uint8_t adcEvents = FALSE;
static uint16_t freetimer = 0;
static uint8_t buttonsEvents = BUTTON_EVENT_NONE;
static uint16_t adcValue;
static uint16_t TIMER_TICK = FALSE;
static uint16_t storetime = 0;
static uint16_t ticks = 0;
static uint16_t storetemp;
static uint16_t inverted = FALSE;
static uint16_t Leds;
static uint16_t leftover;
static int counter = 0;
static int firstASR = TRUE;
static char start;

// **** Define any module-level, global, or external variables here ****
#define TICKS 5
#define LONG_PRESS 5
#define CHARACTERS 6
#define DEFAULT 350
#define ALL_LEDS_OFF 0x00
#define ALL_LEDS_ON 0xFF
#define BROIL_DEFAULT 500
#define TIME 60
#define MASK 0x03FC
// **** Put any helper functions here ****

/*This function will update your OLED to reflect the state .*/
void updateOvenOLED(OvenovenData ovenData) {
    //this variable holds everything that is going to get printed out
    //update OLED here
    // then the rest hold all the characters that are shown when the different
    // modes are all cooking
    char display[TIME] = "";
    
    char onTop[CHARACTERS];
    sprintf(onTop, "|%s%s%s%s%s|", OVEN_TOP_ON, OVEN_TOP_ON, OVEN_TOP_ON, OVEN_TOP_ON, OVEN_TOP_ON);
    char offTop[CHARACTERS];
    sprintf(offTop, "|%s%s%s%s%s|", OVEN_TOP_OFF, OVEN_TOP_OFF, OVEN_TOP_OFF, OVEN_TOP_OFF, OVEN_TOP_OFF);

    char bottomOn[CHARACTERS];
    sprintf(bottomOn, "|%s%s%s%s%s|", OVEN_BOTTOM_ON, OVEN_BOTTOM_ON, OVEN_BOTTOM_ON, OVEN_BOTTOM_ON, OVEN_BOTTOM_ON);

    char bottomOff[CHARACTERS];
    sprintf(bottomOff, "|%s%s%s%s%s|", OVEN_BOTTOM_OFF, OVEN_BOTTOM_OFF, OVEN_BOTTOM_OFF, OVEN_BOTTOM_OFF, OVEN_BOTTOM_OFF);

    // displaying the correct "image" depending on the mode that the user selects
    // if the oven is cooking or reset pending we display the correct things
    // if it is not you show the different in the top and bottom but if the 
    // temperature is being modified you add the ">"
    // getting the different times by dividing by 60 to get a minute 
    // and % by 60 to get the seconds
    switch (ovenData.modes) {
        case BAKE:
            if (ovenData.state == COOKING || ovenData.state == RESET_PENDING) {
                sprintf(display, "%s MODE: BAKE\n|     |  TIME: %d:%02d\n|-----|  TEMP: %d%s""\n%s", onTop, ovenData.cookingTleft / TIME, ovenData.cookingTleft % TIME, ovenData.temp, DEGREE_SYMBOL, bottomOn);

            } else if (!(ovenData.state == COOKING || ovenData.state == RESET_PENDING)) {
                if (!temp_move) {
                    sprintf(display, "%s MODE: BAKE\n|     | >TIME: %d:%02d\n|-----|  TEMP: %d%s" "\n%s", offTop, ovenData.initialCtime / TIME, ovenData.initialCtime % TIME, ovenData.temp, DEGREE_SYMBOL, bottomOff);
                } else {
                    sprintf(display, "%s MODE: BAKE\n|     |  TIME: %d:%02d\n|-----|  >TEMP: %d%s""\n%s", offTop, ovenData.cookingTleft / TIME, ovenData.cookingTleft % TIME, ovenData.temp, DEGREE_SYMBOL, bottomOff);
                }
            }
            break;
        case TOAST:
            if (ovenData.state == COOKING || ovenData.state == RESET_PENDING) {
                sprintf(display, "%s MODE: TOAST\n|     |  TIME: %d:%02d\n|-----|""\n%s", offTop, ovenData.cookingTleft / TIME, ovenData.cookingTleft % TIME,bottomOn);
            } else {
                sprintf(display, "%s MODE: TOAST\n|     |  TIME: %d:%02d\n|-----|""\n%s", offTop, ovenData.initialCtime / TIME, ovenData.initialCtime % TIME,bottomOff);

            }
            break;
        case BROIL:
            if (ovenData.state == COOKING || ovenData.state == RESET_PENDING) {
                sprintf(display, "%s MODE: BROIL\n|     |  TIME: %d:%02d\n|-----|  TEMP: 500%s"
                        "\n%s", onTop, ovenData.cookingTleft / TIME, ovenData.cookingTleft % TIME, DEGREE_SYMBOL, bottomOff);
            } else {
                sprintf(display, "%s MODE: BROIL\n|     |  TIME: %d:%02d\n|-----|  TEMP: 500%s"
                        "\n%s", offTop, ovenData.initialCtime / TIME, ovenData.initialCtime % TIME, DEGREE_SYMBOL, bottomOff);

            }
            break;
    }
    //clearing the screen on the kit after determining what to print on it
    // then actually drawing the correct screen
    // for the EC inverting the display and making it flash
    OledClear(OLED_COLOR_BLACK);
    OledDrawString(display);
    if (ovenData.state == EC) {
        if (inverted) {
            OledSetDisplayNormal();
        } else {
            OledSetDisplayInverted();
        }
    }

    //update OLED here
    OledUpdate();
}

/*This function will execute your state machine.  
 * It should ONLY run if an event flag has been set.*/
void runOvenSM(void) {
    //write your SM logic here.
    switch (data.state) {
        case SETUP:
            // making it so every time the oven is initialized
            // the initial time is set to one second
            if(firstASR == TRUE){
                firstASR = FALSE;
                data.initialCtime = 1;
            }else{
            // getting the adcValue by using the AdcReadFunction 
            // then I bit masked it and then shifted it by two 
            // I did this in order to get the top 8 bits
            // then I added 300
            if (adcEvents) {
                adcValue = AdcRead();
                adcValue = (adcValue & MASK) >> 2;

                if (data.modes == BAKE && temp_move) {
                    data.temp = adcValue + 300;
                } else {
                    data.initialCtime = adcValue + 1;
                    data.cookingTleft = data.initialCtime;
                }
                updateOvenOLED(data);
            }
            }
            
            
            // if the third button is pressed either there will be a change
            // in time or in temperature so the selector changes
            // store the time so that you can check how long the button is 
            // being pressed for

            if (buttonsEvents & BUTTON_EVENT_3DOWN) {
                data.buttonPtime = freetimer;
                data.state = SELECTOR_CHANGE_PENDING;
            }
            // if the fourth button is pressed down then the state changes
            // we are now in cooking state
            // store the time and also turn on all the LEDS
            // the leds turn off based on the initial cook time
            // setting the time that the leds turn off by multiplying by
            // 5 so it turns to seconds and then dividing by 8 like the lab do says to             
            if (buttonsEvents & BUTTON_EVENT_4DOWN) {
                data.state = COOKING;
                storetime = freetimer;
                updateOvenOLED(data);
                LEDS_SET(ALL_LEDS_ON);
                Leds = (data.initialCtime * TICKS) / 8;
                leftover = (data.initialCtime * 5) % 8;
                ticks = 0;
            }

            break;
        case COOKING:
            // if the timer ticks add one to the timer
            // checking how fast or how to make the LEDS move as time goes on
            // adding one to the amount of leds because not every number can be
            // divided by 8 so it helps with getting the time as close to 1/8
            // if the leftover is equal to the amount of ticks that have passed that 
            // means that we need to turn it off
            if (TIMER_TICK) {
                ticks += 1;
                if ((leftover > 0 && ticks == Leds + 1) || (leftover == 0 && ticks == Leds)) {
                    start = LEDS_GET();
                    ticks = 0;
                    LEDS_SET(start << 1);
                }
                // if the cooking time left is 0 then reset everything and make the screen
                // start flashing for the extra credit
                // then turn off all the LEDS
                if (data.cookingTleft == 0) {
                    data.cookingTleft = data.initialCtime;
                    data.state = EC;
                    updateOvenOLED(data);
                    LEDS_SET(ALL_LEDS_OFF);
                    break;
                }
                // checking to see how much time has passed if a TICK or a second
                // then you want to take one away from the cooking time left
                if ((freetimer - storetime) % TICKS == 0) {
                    data.cookingTleft -= 1;
                    updateOvenOLED(data);
                }
            }
            // if the fourth button is pressed while cooking then you want 
            // to reset everything and put the time back to where you started
            if (buttonsEvents & BUTTON_EVENT_4DOWN) {
                data.state = RESET_PENDING;
                data.buttonPtime = freetimer;
            }
            break;
        case SELECTOR_CHANGE_PENDING:
            // if the button 3 is released was released that means that the user
            // pressed the button, if so we have to check how long it was pressed 
            // and what we have to do 
            // also controlling how the toaster travels through the different modes
            if (buttonsEvents & BUTTON_EVENT_3UP) {
                if ((freetimer - data.buttonPtime) >= LONG_PRESS) {
                    if (data.modes == BAKE) {
                        if (!temp_move) {
                            temp_move = TRUE;
                        } else {
                            temp_move = FALSE;
                        }
                    }
                    updateOvenOLED(data);
                    data.state = SETUP;
                } else {
                    if (data.modes == BROIL) {
                        data.modes = BAKE;
                    } else {
                        data.modes += 1;
                    }
                    //adding all the default values to broil and storing the temperature when baking
                    if (data.modes == BROIL) {
                        storetemp = data.temp;
                        data.temp = BROIL_DEFAULT;
                    } else if (data.modes == BAKE) {
                        data.temp = storetemp;
                    }
                    updateOvenOLED(data);
                    data.state = SETUP;
                }
            }
            break;
        case RESET_PENDING:
            if (TIMER_TICK) {
                ticks += 1;
                //checking to see if a second has passed if so then the 
                // Oled needs to be updated
                // while a button is being pressed in this instance
                // that is why this is similar to cooking state
                if ((freetimer - storetime) % TICKS == 0) {
                    data.cookingTleft -= 1;
                    updateOvenOLED(data);
                }
                //checking/moving the led according to how much time has passed.
                if ((leftover == 0 && ticks == Leds) || (leftover > 0 && ticks == Leds + 1)) {
                    start = LEDS_GET();
                    ticks = 0;
                    LEDS_SET(start << 1);
                }
                // when all the time has run out and is less than one second
                // turn off all the LEDS and turn everything back to SETUP
                if (freetimer - data.buttonPtime >= TICKS) {
                    data.cookingTleft = data.initialCtime;
                    data.state = SETUP;
                    updateOvenOLED(data);
                    LEDS_SET(ALL_LEDS_OFF);
                    break;
                }
                //when button 4 is released in the correct amount of time 
                // that means that the toaster oven has to begin cooking
                if (buttonsEvents & BUTTON_EVENT_4UP && (freetimer - data.buttonPtime < TICKS)) {
                    data.state = COOKING;
                }
                break;
            case EC:
                // if the timer ticks or the time runs out then the screen has to be inverted
                // counting how many times the timer ticks
                // could not get it exactly to 2hz
                // my screen flashes just a little faster than the demo
                // I used both 3 and 2 for my conditional but I couldn't get it to the
                // exact speed of the demo, I ended up using 2 as it resembles me 
                // trying to get 2hz
                // when button 4 is pressed that end the flashing and just resets the 
                // toaster oven back to setup mode.
                if(TIMER_TICK){
                    counter++;
                }
                if (counter == 2) {
                    if (!inverted) {
                        inverted = TRUE;
                    } else {
                        inverted = FALSE;
                    }
                    updateOvenOLED(data);
                    counter = 0;
                }
                if (buttonsEvents & BUTTON_EVENT_4UP) {
                    inverted = TRUE;
                    updateOvenOLED(data);
                    data.state = SETUP;
                    updateOvenOLED(data);
                }
                break;                    
            }
    }
}

int main() {
    BOARD_Init();

    //initalize timers and timer ISRs:
    // <editor-fold defaultstate="collapsed" desc="TIMER SETUP">

    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a .01s timer.

    T2CON = 0; // everything should be off
    T2CONbits.TCKPS = 0b100; // 1:16 prescaler
    PR2 = BOARD_GetPBClock() / 16 / 100; // interrupt at .5s intervals
    T2CONbits.ON = 1; // turn the timer on

    // Set up the timer interrupt with a priority of 4.
    IFS0bits.T2IF = 0; //clear the interrupt flag before configuring
    IPC2bits.T2IP = 4; // priority of  4
    IPC2bits.T2IS = 0; // subpriority of 0 arbitrarily 
    IEC0bits.T2IE = 1; // turn the interrupt on

    // Configure Timer 3 using PBCLK as input. We configure it using a 1:256 prescaler, so each timer
    // tick is actually at F_PB / 256 Hz, so setting PR3 to F_PB / 256 / 5 yields a .2s timer.

    T3CON = 0; // everything should be off
    T3CONbits.TCKPS = 0b111; // 1:256 prescaler
    PR3 = BOARD_GetPBClock() / 256 / 5; // interrupt at .5s intervals
    T3CONbits.ON = 1; // turn the timer on

    // Set up the timer interrupt with a priority of 4.
    IFS0bits.T3IF = 0; //clear the interrupt flag before configuring
    IPC3bits.T3IP = 4; // priority of  4
    IPC3bits.T3IS = 0; // subpriority of 0 arbitrarily 
    IEC0bits.T3IE = 1; // turn the interrupt on;

    // </editor-fold>

    printf("Welcome to ahern336's Lab07 (Toaster Oven).  Compiled on %s %s.", __TIME__, __DATE__);

    //initialize state machine (and anything else you need to init) here
    // making the state and mode start at BAKE and SETUP
    data.initialCtime = 1;
    data.cookingTleft = 1;
    data.buttonPtime = 0;
    data.temp = DEFAULT;
    data.state = SETUP;
    data.modes = BAKE;

    OledInit();
    ButtonsInit();
    AdcInit();
    LEDS_INIT();

    updateOvenOLED(data);
    while (1) {
        // Add main loop code here:
        // check for events
        // on event, run runOvenSM()
        // clear event flags
        if (adcEvents || TIMER_TICK || buttonsEvents != BUTTON_EVENT_NONE) {
            runOvenSM();
            buttonsEvents = BUTTON_EVENT_NONE;
            adcEvents = FALSE;
            TIMER_TICK = FALSE;
        }
    };
}

/*The 5hz timer is used to update the free-running timer and to generate TIMER_TICK events*/
void __ISR(_TIMER_3_VECTOR, ipl4auto) TimerInterrupt5Hz(void) {
    // Clear the interrupt flag.
    IFS0CLR = 1 << 12;

    //add event-checking code here
    TIMER_TICK = TRUE;
    freetimer += 1;
}

/*The 100hz timer is used to check for button and ADC events*/
void __ISR(_TIMER_2_VECTOR, ipl4auto) TimerInterrupt100Hz(void) {
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;

    //add event-checking code here
    adcEvents = AdcChanged();
    buttonsEvents = ButtonsCheckEvents();
}
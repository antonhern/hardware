//Antonio Hernandez Ruiz
// libraries
#include <stdint.h>
#include <genericTypeDefs.h>

//header files
#include "BOARD.h"
#include "Buttons.h"

//static variables
static uint8_t debounce;
static uint8_t start;

void ButtonsInit(void){
    TRISD |= 0x00E0;
    TRISF |= 0x0002;
    //variables to hold the debounce amount and the starting button event
    debounce = 0;
    start = 0x00;
}

uint8_t ButtonsCheckEvents(void){
    //at the beginning there is no button event because nothing has been pressed or de bounced
  uint8_t beginning = BUTTON_EVENT_NONE; 
  uint8_t status = BUTTON_STATES();
  
  // if the state of the button is the start or the first move then don't do anything
  if(status == start){
      return BUTTON_EVENT_NONE;
  }
  // nothing happens if the debounce is less than zero
  if(debounce > 0){
      debounce -=1;
      return beginning;
  }
  //checking the previous state of the button to the current state that it has now
  //if the button has changed to go down the value is going to be 0x00
  //then if that is the case it is a button down
  //when the button is up the status is going to not be button state but
  //the start or previous is
  if((status & BUTTON_STATE_1) && ((start & BUTTON_STATE_1)== 0x00)){
      beginning |= BUTTON_EVENT_1DOWN;
  }else if(!(status & BUTTON_STATE_1) && start & BUTTON_STATE_1){
      beginning |= BUTTON_EVENT_1UP;
  }
  if((status & BUTTON_STATE_2) && ((start & BUTTON_STATE_2)== 0x00)){
      beginning |= BUTTON_EVENT_2DOWN;
  }else if(!(status & BUTTON_STATE_2) && start & BUTTON_STATE_2){
      beginning |= BUTTON_EVENT_2UP;
  }
  if((status & BUTTON_STATE_3) && ((start & BUTTON_STATE_3) == 0x00)){
      beginning |= BUTTON_EVENT_3DOWN;
  }else if(!(status & BUTTON_STATE_3) && start & BUTTON_STATE_3){
      beginning |= BUTTON_EVENT_3UP;
  }
  if((status & BUTTON_STATE_4) && ((start & BUTTON_STATE_4)==0x00)){
      beginning |= BUTTON_EVENT_4DOWN;
  }else if(!(status & BUTTON_STATE_4) && start & BUTTON_STATE_4){
      beginning |= BUTTON_EVENT_4UP;
  }
  //resetting the debounce period
  debounce = BUTTONS_DEBOUNCE_PERIOD;
  start = status;
  return beginning;
}
// Author: Antonio Hernandez Ruiz

//Header files
#include <stdio.h>
#include <stdlib.h>
#include "Leds_Lab06.h"
#include "BOARD.h"

//function to initialize the led bar
void LEDS_INIT(void){
    TRISE = 0x00;
    LATE = 0x00;
}

void LEDS_SET(char newPattern){
    LATE = newPattern;
}

char LEDS_GET(void){
    return LATE;
}
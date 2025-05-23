// ============= MAIN.C =============
#include <LPC17xx.h>
#include "lcd.h"
#include "keypad.h"
#include "delay.h"
#include "logic.h"

int main(void) 
{
    lcdinit();
    KeyPadInitialize();
    
    while (1) {
        RunCalculatorLogic();
        delay(100); // Brief delay between calculator sessions
    }
}

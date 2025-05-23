// ============= KEYPAD.C =============
#include <LPC17xx.h>
#include "keypad.h"
#include "delay.h"

char keyCodes[4][4] = {
    {0x1, 0x2, 0x3, 0x4},
    {0x5, 0x6, 0x7, 0x8}, 
    {0x9, 0x0, 0xA, 0xB},
    {0xC, 0xD, 0xE, 0xF}
};

void KeyPadInitialize(void)
{
    // Make rows output
    LPC_GPIO1->FIODIR |= (1 << 9) | (1 << 10) | (1 << 14) | (1 << 15);
    // Make columns input  
    LPC_GPIO1->FIODIR &= ~((1 << 0) | (1 << 1) | (1 << 4) | (1 << 8));
    
    // Set all rows high initially
    LPC_GPIO1->FIOSET = (1 << 9) | (1 << 10) | (1 << 14) | (1 << 15);
}

void SetRowToZero(unsigned char rowNumber)
{
    // Set all rows high first
    LPC_GPIO1->FIOSET = (1 << 9) | (1 << 10) | (1 << 14) | (1 << 15);
    
    // Pull selected row low
    switch (rowNumber) {
        case 0: LPC_GPIO1->FIOCLR = (1 << 9); break;
        case 1: LPC_GPIO1->FIOCLR = (1 << 10); break;  
        case 2: LPC_GPIO1->FIOCLR = (1 << 14); break;
        case 3: LPC_GPIO1->FIOCLR = (1 << 15); break;
    }
    delay(1); // Signal stabilization delay
}

unsigned char ReadColumnNumber(void)
{
    unsigned long int temp = LPC_GPIO1->FIOPIN;
    
    if ((temp & (1 << 0)) == 0) return 0;
    if ((temp & (1 << 1)) == 0) return 1; 
    if ((temp & (1 << 4)) == 0) return 2;
    if ((temp & (1 << 8)) == 0) return 3;
    return 4; // No key pressed
}

// FIXED: Non-blocking key detection with improved debouncing
unsigned char GetKeyPressed(void)
{
    static unsigned char lastKey = 0xFF;
    static int debounceCount = 0;
    static int stableCount = 0;
    unsigned char row, col;
    unsigned char currentKey = 0xFF;
    
    // Scan all rows to find pressed key
    for (row = 0; row < 4; row++) {
        SetRowToZero(row);
        col = ReadColumnNumber();
        if (col < 4) {
            currentKey = keyCodes[row][col];
            break; // Found a key, stop scanning
        }
    }
    
    // Debouncing logic
    if (currentKey != 0xFF) { // Key is pressed
        if (currentKey == lastKey) {
            stableCount++;
            if (stableCount >= 5) { // Key stable for 5 consecutive reads
                if (debounceCount == 0) { // First time registering this key
                    debounceCount = 1;
                    return currentKey; // Return the key
                }
                // Key is being held - don't return it again
                return 0xFF;
            }
        } else {
            // Different key or first detection
            lastKey = currentKey;
            stableCount = 1;
            debounceCount = 0;
        }
    } else {
        // No key pressed - reset everything
        if (lastKey != 0xFF) {
            // Key was just released
            lastKey = 0xFF;
            stableCount = 0;
            debounceCount = 0;
        }
    }
    
    return 0xFF; // No valid key or still debouncing
}

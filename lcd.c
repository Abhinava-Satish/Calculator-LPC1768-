// ============= LCD.C (FIXED VERSION) =============
#include <LPC17xx.h>
#include "lcd.h"
#include "delay.h"

#define RS 9
#define RW 10
#define EN 11

void lcdinit(void) 
{
    // Set data pins and control pins as output
    LPC_GPIO0->FIODIR |= (1 << 22) | (1 << 21) | (1 << 20) | (1 << 19);
    LPC_GPIO0->FIODIR |= (1 << EN) | (1 << RW) | (1 << RS);
    
    // Initialize all pins to low
    LPC_GPIO0->FIOCLR = (1 << RS) | (1 << RW) | (1 << EN);
    LPC_GPIO0->FIOCLR = (0x0F << 19); // Clear data pins
    
    delay(100); // Power-on delay - increased from 50ms
    
    // LCD initialization sequence for 4-bit mode
    // Send 0x30 three times with proper delays
    LPC_GPIO0->FIOCLR = (0x0F << 19);   // Clear data pins
    LPC_GPIO0->FIOSET = (0x03 << 19);   // Set 0x3 on upper nibble
    LPC_GPIO0->FIOSET = (1 << EN);      // Enable high
    delay(5);                           // Longer enable pulse
    LPC_GPIO0->FIOCLR = (1 << EN);      // Enable low
    delay(10);                          // Wait 10ms
    
    // Second 0x30
    LPC_GPIO0->FIOSET = (1 << EN);
    delay(5);
    LPC_GPIO0->FIOCLR = (1 << EN);
    delay(5);
    
    // Third 0x30  
    LPC_GPIO0->FIOSET = (1 << EN);
    delay(5);
    LPC_GPIO0->FIOCLR = (1 << EN);
    delay(5);
    
    // Now send 0x20 to set 4-bit mode
    LPC_GPIO0->FIOCLR = (0x0F << 19);
    LPC_GPIO0->FIOSET = (0x02 << 19);   // 0x2 for 4-bit mode
    LPC_GPIO0->FIOSET = (1 << EN);
    delay(5);
    LPC_GPIO0->FIOCLR = (1 << EN);
    delay(10);
    
    // Now use 4-bit commands
    lcdchar(0x28, 'C'); delay(10);  // 4-bit, 2 lines, 5x7 font
    lcdchar(0x0C, 'C'); delay(10);  // Display ON, Cursor OFF, Blink OFF
    lcdchar(0x06, 'C'); delay(10);  // Increment cursor, no shift
    lcdchar(0x01, 'C'); delay(20);  // Clear display - needs longer delay
}

void lcdstring(char *str) 
{
    unsigned char i = 0;
    while (str[i] != '\0') {
        lcdchar(str[i], 'D');
        delay(2); // Small delay between characters
        i++;
    }
}

void lcdchar(unsigned char data, unsigned char type) 
{
    unsigned char lower = data & 0x0F;
    unsigned char upper = (data & 0xF0) >> 4;
    
    // Set command/data mode
    if (type == 'C') 
        LPC_GPIO0->FIOCLR = (1 << RS);  // Command mode
    else 
        LPC_GPIO0->FIOSET = (1 << RS);  // Data mode
    
    // Ensure RW is low (write mode)
    LPC_GPIO0->FIOCLR = (1 << RW);
    
    delay(1); // Setup time
    
    // Send upper nibble
    LPC_GPIO0->FIOCLR = (0x0F << 19);   // Clear data pins
    LPC_GPIO0->FIOSET = (upper << 19);  // Set upper nibble
    LPC_GPIO0->FIOSET = (1 << EN);      // Enable high
    delay(5);                           // Enable pulse width - increased
    LPC_GPIO0->FIOCLR = (1 << EN);      // Enable low
    delay(5);                           // Hold time - increased
    
    // Send lower nibble  
    LPC_GPIO0->FIOCLR = (0x0F << 19);   // Clear data pins
    LPC_GPIO0->FIOSET = (lower << 19);  // Set lower nibble
    LPC_GPIO0->FIOSET = (1 << EN);      // Enable high
    delay(5);                           // Enable pulse width - increased
    LPC_GPIO0->FIOCLR = (1 << EN);      // Enable low
    delay(5);                           // Hold time - increased
    
    // Extra delay for commands
    if (type == 'C') {
        if (data == 0x01 || data == 0x02) {
            delay(20); // Clear/Home commands need more time
        } else {
            delay(5);  // Other commands
        }
    } else {
        delay(2); // Data write delay
    }
}

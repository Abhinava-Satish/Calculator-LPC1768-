// test_stubs.c - Stub implementations for unit testing logic.c

#include "lcd.h"
#include "keypad.h"
#include "delay.h"
#include <stdio.h> // For printf in stubs if needed for debugging

// --- LCD Stubs ---
void lcdchar(unsigned char data, unsigned char type) {
    // Stub: Can print to console for debugging if needed
    // printf("lcdchar: data=0x%X, type=%c\n", data, type);
}

void lcdstring(char *str) {
    // Stub: Can print to console for debugging if needed
    // printf("lcdstring: \"%s\"\n", str);
}

void lcdinit(void) {
    // Stub: Does nothing
}

// --- Keypad Stubs ---
// Global variable to control GetKeyPressed return value for specific tests if needed
static unsigned char mock_key_pressed = KEY_NONE; 

void SetMockKeyPressed(unsigned char key) {
    mock_key_pressed = key;
}

unsigned char GetKeyPressed(void) {
    // Stub: Returns a pre-set value or KEY_NONE
    unsigned char key_to_return = mock_key_pressed;
    if (mock_key_pressed != KEY_NONE) {
        // Optional: Automatically reset to KEY_NONE after one read to simulate real keypad
        // mock_key_pressed = KEY_NONE; 
    }
    return key_to_return;
}

// --- Delay Stubs ---
void delay(unsigned int ms) {
    // Stub: Does nothing, or could simulate time passing if tests become time-sensitive
}

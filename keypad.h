// ============= KEYPAD.H =============
#ifndef KEYPAD_H
#define KEYPAD_H

void KeyPadInitialize(void);
void SetRowToZero(unsigned char rowNumber);
unsigned char ReadColumnNumber(void);
unsigned char GetKeyPressed(void);

#define MAX_TOKENS 50

#endif

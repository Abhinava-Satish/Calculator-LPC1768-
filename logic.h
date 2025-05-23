// ============= LOGIC.H =============
#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h> // For bool type

// --- Configuration Constants ---
#define MAX_TOKENS 50      // Maximum number of tokens (numbers/operators) in an expression
#define ERROR_MSG_LEN 17   // Max 16 displayable chars for LCD error messages + null terminator
#define LCD_LINE_LEN 16    // Character width of the LCD display line

// --- LCD Command Codes (used in logic.c, though ideally part of lcd driver) ---
#define LCD_CMD_CLEAR_DISPLAY 0x01
#define LCD_CMD_CURSOR_LINE_2 0xC0 // Command to move cursor to the beginning of the second line

// --- Key Code Definitions (from keypad) ---
#define KEY_0         0x0
#define KEY_1         0x1
#define KEY_2         0x2
#define KEY_3         0x3
#define KEY_4         0x4
#define KEY_5         0x5
#define KEY_6         0x6
#define KEY_7         0x7
#define KEY_8         0x8
#define KEY_9         0x9
#define KEY_PLUS      0xA
#define KEY_MINUS     0xB
#define KEY_MULTIPLY  0xC
#define KEY_DIVIDE    0xD
#define KEY_EQUALS    0xE
#define KEY_DECIMAL   0xF
#define KEY_NONE      0xFF // Value indicating no key is pressed

// --- Global Error State ---
// These variables are defined in logic.c and used to manage error conditions.
extern bool calculator_error; // True if an error has occurred
extern char error_message[ERROR_MSG_LEN]; // Stores the current error message string

// --- Public Function Prototypes ---

/**
 * @brief Main function to run the calculator's logic loop.
 * 
 * This function initializes the calculator state and enters an infinite loop
 * to process key presses, manage display updates, handle calculations,
 * and manage error states.
 */
void RunCalculatorLogic(void);


// --- Declarations for functions primarily used within logic.c but exposed for potential testing ---
// These are not strictly part of the public API for other modules but are declared
// to allow direct testing from a test harness.

/**
 * @brief Parses the string `current_num_str` into a float.
 * 
 * Handles positive/negative numbers and decimal points.
 * Sets `calculator_error` if the number format is invalid (e.g., multiple decimal points).
 * @return The parsed floating-point number, or 0.0f if an error occurs during parsing.
 */
float parse_current_input_number(void);

/**
 * @brief Evaluates the expression stored in `expr_type` and `expr_data`.
 * 
 * Uses a shunting-yard-like approach with two stacks (one for values, one for operators)
 * to handle operator precedence and compute the result.
 * Sets `calculator_error` for syntax errors, division by zero, or stack overflows/underflows.
 * @return The calculated result, or 0.0f if an error occurs.
 */
float evaluate_full_expression(void);

/**
 * @brief Clears all calculator state variables and resets any error conditions.
 * 
 * Resets expression arrays, current number input, display strings, and error flags.
 */
void clear_all_state(void);

/**
 * @brief Sets the global error flag and stores the error message.
 * 
 * If an error is already set, this function does nothing to preserve the first error.
 * @param message The error message string to store and display.
 */
void set_error(const char* message);


#endif // LOGIC_H

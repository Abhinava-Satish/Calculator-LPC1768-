// ============= LOGIC.C =============
// Core logic for the calculator application.
// Handles user input, expression parsing, calculation, display updates, and error management.
// ===================================

#include "logic.h"
#include "keypad.h" // For GetKeyPressed()
#include "delay.h"  // For delay()
#include "lcd.h"    // For lcdchar(), lcdstring()

#include <stdio.h>   // For snprintf()
#include <stdbool.h> // For bool type
#include <math.h>    // For fabsf(), roundf()
#include <string.h>  // For strlen(), strncpy(), strcat(), memset()

// --- Defines ---
#define MAX_DISPLAY_STR 32  // Max length for the full expression string history (not directly displayed fully)
#define FLOAT_EPSILON 1e-7f // Epsilon for comparing float to integer and for division by zero check

// --- Global Variables ---
// Error State
bool calculator_error = false;           // Flag: true if an error is currently active
char error_message[ERROR_MSG_LEN] = {0}; // Buffer to store the current error message string

// Expression Storage (Shunting-Yard Intermediate Representation)
char expr_type[MAX_TOKENS];  // Array to store the type of each token ('N' for number, 'O' for operator)
float expr_data[MAX_TOKENS]; // Array to store the value of numbers or the char code of operators
int expr_len = 0;            // Current number of tokens in the expression

// Display Strings
char expression_str[MAX_DISPLAY_STR];     // Stores the full infix expression string as entered by the user (for history/scrolling)
int expression_index = 0;                 // Current length of the expression_str
char current_num_str[LCD_LINE_LEN + 1]; // Stores the string for the number currently being typed by the user
int current_num_index = 0;                // Current length of current_num_str

/**
 * @brief Sets the global error flag and stores the error message.
 * 
 * If an error is already set (calculator_error is true), this function does nothing,
 * preserving the first error that occurred.
 * @param message The error message string to store. It will be truncated if longer than
 *                ERROR_MSG_LEN - 1.
 */
void set_error(const char* message) {
    if (calculator_error) {
        return; // Preserve the first error encountered
    }
    calculator_error = true;
    strncpy(error_message, message, ERROR_MSG_LEN - 1);
    error_message[ERROR_MSG_LEN - 1] = '\0'; // Ensure null termination
}

/**
 * @brief Clears all calculator state variables and resets any error conditions.
 * 
 * This function is called at startup and when a new calculation is initiated
 * after a previous one has ended (either with a result or an error).
 */
void clear_all_state() { 
    expr_len = 0;
    expression_index = 0;
    current_num_index = 0;

    memset(expression_str, 0, sizeof(expression_str));
    memset(current_num_str, 0, sizeof(current_num_str));
    // expr_type and expr_data do not need explicit clearing as their usage is governed by expr_len.
    
    calculator_error = false;
    error_message[0] = '\0';
}

/**
 * @brief Updates the LCD display with the current expression string and current number input.
 * 
 * Line 1 of the LCD shows the a portion of the historical expression string.
 * Line 2 of the LCD shows the number currently being typed.
 * This function is typically called after each key press that modifies the input.
 * It does not handle error message display; that's done in RunCalculatorLogic.
 */
void update_lcd_display_content() { 
    char display_buffer_line1[LCD_LINE_LEN + 1] = {0};
    char display_buffer_line2[LCD_LINE_LEN + 1] = {0};
    int start_pos;

    // Prepare Line 1: Historical expression string (last LCD_LINE_LEN characters)
    if (expression_index > 0) {
        start_pos = (expression_index > LCD_LINE_LEN) ? (expression_index - LCD_LINE_LEN) : 0;
        strncpy(display_buffer_line1, &expression_str[start_pos], LCD_LINE_LEN);
    }

    // Prepare Line 2: Current number being typed
    if (current_num_index > 0) {
        strncpy(display_buffer_line2, current_num_str, LCD_LINE_LEN);
    }
    // If no current number and no expression history, Line 2 remains blank.
    
    lcdchar(LCD_CMD_CLEAR_DISPLAY, 'C'); // 'C' for command
    delay(20); // Delay for LCD command processing
    lcdstring(display_buffer_line1);
    lcdchar(LCD_CMD_CURSOR_LINE_2, 'C'); 
    delay(5); // Delay for LCD command processing
    lcdstring(display_buffer_line2);
}

/**
 * @brief Appends a string (typically a number or operator) to the historical expression string.
 * 
 * This string (`expression_str`) is used by `update_lcd_display_content` to show
 * the expression history on the first line of the LCD.
 * @param str_to_add The string to append.
 */
void add_to_expression_string(const char* str_to_add) {
    int len_to_add = strlen(str_to_add);
    if (expression_index + len_to_add < MAX_DISPLAY_STR - 1) { // Check against buffer size
        strcat(expression_str, str_to_add);
        expression_index += len_to_add;
    }
    // If buffer is full, further additions are ignored to prevent overflow.
}

/**
 * @brief Pushes a number (operand) onto the internal expression stack.
 * 
 * Sets an error if an error is already active or if the expression token limit is reached.
 * @param num The floating-point number to push.
 */
void push_operand_to_expr(float num) {
    if (calculator_error) {
        return;
    }
    if (expr_len >= MAX_TOKENS) {
        set_error("Err: Expr Long"); 
        return;
    }
    expr_type[expr_len] = 'N'; // 'N' for Number
    expr_data[expr_len] = num;
    expr_len++;
}

/**
 * @brief Pushes an operator onto the internal expression stack and its string to display history.
 * 
 * Sets an error if an error is already active or if the expression token limit is reached.
 * @param op_char The character representing the operator (e.g., '+', '-').
 */
void push_operator_to_expr(char op_char) {
    if (calculator_error) {
        return;
    }
    if (expr_len >= MAX_TOKENS) {
        set_error("Err: Expr Long");
        return;
    }
    expr_type[expr_len] = 'O'; // 'O' for Operator
    expr_data[expr_len] = (float)op_char; // Store operator char as a float for type consistency in expr_data
    expr_len++;
    
    char op_str[2] = {op_char, '\0'};
    add_to_expression_string(op_str); // Add operator to the display string
}

/**
 * @brief Determines the precedence of an arithmetic operator.
 * 
 * Higher return value means higher precedence. Used by `evaluate_full_expression`.
 * @param op The operator character.
 * @return Precedence level (1 for +/-, 2 for */, 0 otherwise).
 */
int get_precedence(char op) {
    if (op == '+' || op == '-') {
        return 1;
    }
    if (op == '*' || op == '/') {
        return 2;
    }
    return 0; // Should not happen for valid operators
}

/**
 * @brief Applies a binary arithmetic operator to two operands.
 * 
 * Sets "Err: Div Zero" if division by zero is attempted.
 * Sets "Err: Syntax" for unknown operators (should not occur with valid input).
 * @param op The operator character.
 * @param a The first (left) operand.
 * @param b The second (right) operand.
 * @return The result of the operation, or 0.0f if an error occurs.
 */
float execute_apply_operator(char op, float a, float b) {
    if (calculator_error) { // Should ideally be checked before calling
        return 0.0f; 
    }

    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': 
            // Check for division by zero (positive or negative zero)
            if (fabsf(b) < FLOAT_EPSILON) { 
                set_error("Err: Div Zero");
                return 0.0f; 
            }
            return a / b;
        default: 
            // This case should ideally not be reached if input validation is correct
            set_error("Err: Syntax"); 
            return 0.0f;
    }
}

/**
 * @brief Evaluates the current expression stored in `expr_type` and `expr_data`.
 * 
 * This function implements a simplified version of the shunting-yard algorithm
 * (or rather, direct evaluation using two stacks for numbers and operators)
 * to handle operator precedence.
 * - It iterates through the tokenized expression (`expr_type`, `expr_data`).
 * - Numbers are pushed onto a value stack.
 * - Operators are pushed onto an operator stack, maintaining precedence rules:
 *   If the current operator has lower or equal precedence than the operator
 *   at the top of the operator stack, the stack's top operator is applied to
 *   values from the value stack, and the result is pushed back to the value stack.
 * - After processing all tokens, any remaining operators on the stack are applied.
 * - The final result should be the single remaining value on the value stack.
 * 
 * Sets various errors ("Err: Syntax", "Err: Stack") if issues are found during evaluation.
 * @return The calculated result of the expression, or 0.0f if an error occurs.
 */
float evaluate_full_expression() {
    if (calculator_error) { // If error already set (e.g. during input parsing)
        return 0.0f; 
    }
    if (expr_len == 0) { // Empty expression
        return 0.0f; 
    }

    // Check for trailing operator: e.g. "5 + ="
    // An expression like "5 =" (expr_len=1, type='N') is valid.
    if (expr_len > 0 && expr_type[expr_len - 1] == 'O') {
        // This check ensures that an operator is not the last thing in a multi-token expression.
        // A single number is fine (expr_len=1, expr_type[0]=='N').
        // A single operator (expr_len=1, expr_type[0]=='O') would also be caught here if not desired.
        // However, current input logic aims to prevent pushing just an operator if expr_len is 0.
        set_error("Err: Syntax"); 
        return 0.0f;
    }
    
    // If only a single number was pushed (e.g., "5="), return that number.
    if (expr_len == 1 && expr_type[0] == 'N') {
        return expr_data[0];
    }

    float val_stack[MAX_TOKENS]; // Stack for numbers/operands
    char op_stack[MAX_TOKENS];   // Stack for operator characters
    int val_top = -1, op_top = -1;

    for (int i = 0; i < expr_len; i++) {
        if (expr_type[i] == 'N') { // Token is a number
            if (val_top >= MAX_TOKENS - 1) { // Value stack overflow
                set_error("Err: Stack"); 
                return 0.0f;
            }
            val_stack[++val_top] = expr_data[i];
        } else { // Token is an operator
            char current_op_char = (char)expr_data[i];
            // While operator stack is not empty, and top operator has higher or equal precedence
            while (op_top >= 0 && get_precedence(op_stack[op_top]) >= get_precedence(current_op_char)) {
                if (val_top < 1) { // Not enough operands on value stack
                    set_error("Err: Syntax"); 
                    return 0.0f; 
                }
                char op_to_apply = op_stack[op_top--];
                float b_val = val_stack[val_top--];
                float a_val = val_stack[val_top--];
                val_stack[++val_top] = execute_apply_operator(op_to_apply, a_val, b_val);
                if (calculator_error) { // Check if execute_apply_operator set an error (e.g., Div Zero)
                    return 0.0f; 
                }
            }
            // Push current operator onto operator stack
            if (op_top >= MAX_TOKENS - 1) { // Operator stack overflow
                set_error("Err: Stack"); 
                return 0.0f;
            }
            op_stack[++op_top] = current_op_char;
        }
    }

    // After all tokens are processed, apply any remaining operators on the stack
    while (op_top >= 0) {
        if (val_top < 1) { // Not enough operands
            set_error("Err: Syntax"); 
            return 0.0f; 
        }
        char op_to_apply = op_stack[op_top--];
        float b_val = val_stack[val_top--];
        float a_val = val_stack[val_top--];
        val_stack[++val_top] = execute_apply_operator(op_to_apply, a_val, b_val);
        if (calculator_error) {
            return 0.0f;
        }
    }

    // The final result should be the only item left on the value stack
    if (val_top != 0) { 
        set_error("Err: Syntax"); // Should indicate an issue with expression structure or evaluation logic
        return 0.0f;
    }
    return val_stack[val_top];
}

/**
 * @brief Parses the `current_num_str` (string being typed by user) into a float.
 * 
 * This function handles:
 * - Empty string (returns 0.0f).
 * - Standalone "-" or "." (sets "Err: Syntax").
 * - Positive and negative integers and floating-point numbers.
 * - Multiple decimal points (sets "Err: Syntax").
 * - Invalid characters (though input filtering in `RunCalculatorLogic` should prevent this).
 * 
 * @return The parsed floating-point number. If parsing fails, an error is set via `set_error()`,
 *         and this function returns 0.0f.
 */
float parse_current_input_number() {
    if (current_num_index == 0) { // Empty string
        return 0.0f;
    }
    // Check for invalid standalone inputs
    if (current_num_index == 1 && current_num_str[0] == '-') {
        set_error("Err: Syntax"); // Standalone minus is not a valid number to parse
        return 0.0f;
    }
    if (current_num_index == 1 && current_num_str[0] == '.') {
        // Input logic in RunCalculatorLogic should turn "." into "0."
        // If somehow "." reaches here alone, it's a syntax error.
        set_error("Err: Syntax");
        return 0.0f;
    }
    // Note: "123." is a valid input during typing; it's parsed as 123.0.

    float result = 0.0f;
    float decimal_multiplier = 0.1f; // For digits after decimal point
    bool in_decimal_part = false;    // Flag to track if we are parsing the fractional part
    bool is_negative_num = false;
    int start_idx = 0;               // Starting index for parsing (0, or 1 if negative)

    if (current_num_str[0] == '-') {
        is_negative_num = true;
        start_idx = 1;
    }

    for (int i = start_idx; i < current_num_index; i++) {
        if (current_num_str[i] == '.') {
            if (in_decimal_part) { // Already encountered a decimal point
                set_error("Err: Syntax"); 
                return 0.0f; 
            }
            in_decimal_part = true;
        } else if (current_num_str[i] >= '0' && current_num_str[i] <= '9') {
            int digit = current_num_str[i] - '0';
            if (!in_decimal_part) {
                result = result * 10.0f + digit; // Accumulate integer part
            } else {
                result += digit * decimal_multiplier; // Accumulate fractional part
                decimal_multiplier *= 0.1f;
            }
        } else { 
            // Should not be reached if RunCalculatorLogic filters input correctly
            set_error("Err: Syntax"); 
            return 0.0f; 
        }
    }
    return is_negative_num ? -result : result;
}

/**
 * @brief Main operational loop for the calculator.
 * 
 * Initializes the calculator, displays a ready message, and then enters an infinite loop
 * to handle key presses and manage calculator states:
 * 
 * 1.  **Initial State / Ready**: Displays "Calculator Ready".
 * 2.  **Input Processing**:
 *     - Reads key presses from `GetKeyPressed()`.
 *     - Handles digit input (0-9), decimal point, and operators (+, -, *, /).
 *     - Updates `current_num_str` for numbers being typed.
 *     - Appends to `expression_str` for display history.
 *     - Pushes operands and operators to `expr_type`/`expr_data` stacks.
 *     - Sets errors like "Err: Num Len", "Err: Syntax" if input rules are violated.
 * 3.  **Calculation (on KEY_EQUALS)**:
 *     - Parses the final `current_num_str`.
 *     - Calls `evaluate_full_expression()` to compute the result.
 *     - Manages display of the result or any error message from evaluation.
 *     - Sets `calculation_has_ended` to true.
 * 4.  **Error State Management**:
 *     - If `calculator_error` is set at any point (input or calculation):
 *       - Displays the `error_message`.
 *       - Sets `calculation_has_ended` to true.
 *       - The calculator effectively waits in this state.
 * 5.  **Post-Calculation/Error State (`calculation_has_ended == true`)**:
 *     - The result or error message remains on the display.
 *     - Any key press (except KEY_NONE or KEY_EQUALS if an error was shown) will:
 *       - Call `clear_all_state()` to reset everything (including errors).
 *       - Transition back to normal input processing, using the pressed key as the
 *         start of a new calculation if it's a valid input key.
 * 
 * The loop includes small delays for keypad polling and LCD command processing.
 */
void RunCalculatorLogic(void) {
    bool calculation_has_ended = false;    // True if result or error is currently displayed
    bool decimal_point_entered = false;  // Tracks if decimal point is already in current_num_str
    bool last_key_was_operator = false;  // Helps manage operator chaining and unary minus logic
    unsigned char current_key;             // Stores the currently pressed key

    clear_all_state(); // Initialize all states and clear any residual errors
    
    // Initial startup message
    lcdchar(LCD_CMD_CLEAR_DISPLAY, 'C'); 
    delay(20);
    lcdstring("Calculator Ready");
    lcdchar(LCD_CMD_CURSOR_LINE_2, 'C'); 
    delay(5);
    lcdstring("Enter Expression");
    delay(1000); // Display ready message for a second
    update_lcd_display_content(); // Update to initial empty/input state

    // Main calculator loop
    while (true) {
        current_key = GetKeyPressed(); // Poll for a key press

        // State 1: Calculation has ended (result or error is shown)
        if (calculation_has_ended) {
            if (current_key != KEY_NONE) { 
                bool error_was_being_displayed = calculator_error; 
                clear_all_state(); // Reset everything for a new calculation
                calculation_has_ended = false;
                decimal_point_entered = false;
                last_key_was_operator = false;
                
                // If KEY_EQUALS was pressed while an error was shown, it's treated as a clear signal.
                // We don't want to re-process KEY_EQUALS as a command to calculate an empty expression.
                if (current_key == KEY_EQUALS && error_was_being_displayed) { 
                    update_lcd_display_content(); // Show the now-cleared display
                    continue; // Go back to waiting for new input
                }
                // If any other key was pressed, it will fall through to be processed as new input.
            } else {
                delay(50); // No key pressed, continue showing result/error
                continue;
            }
        }
        
        // State 2: Error occurred during input processing (before KEY_EQUALS)
        // This ensures errors like "Num Len" are displayed immediately if another action is attempted.
        if (calculator_error && !calculation_has_ended) {
            if (current_key == KEY_NONE) { 
                delay(50); 
                continue;
            }
            // If an error is set and a key is pressed, transition to error display state
            lcdchar(LCD_CMD_CLEAR_DISPLAY, 'C'); 
            delay(20);
            lcdstring(error_message);
            lcdchar(LCD_CMD_CURSOR_LINE_2, 'C'); 
            delay(5);
            lcdstring(""); // Clear second line
            calculation_has_ended = true; // Enter the "error displayed" state
            continue; // Wait for next key press to clear the error
        }

        // State 3: Normal input processing (no key pressed yet, or processing current key)
        if (current_key == KEY_NONE) {
            delay(50); 
            continue;
        }

        // --- Process Valid Key Presses ---
        if (current_key >= KEY_0 && current_key <= KEY_9) { // Digit keys
            if (current_num_index < LCD_LINE_LEN) { // Prevent overflow of current_num_str
                current_num_str[current_num_index++] = '0' + current_key;
                current_num_str[current_num_index] = '\0';
                last_key_was_operator = false;
            } else {
                set_error("Err: Num Len"); // Number input is too long
            }
        } else if (current_key == KEY_DECIMAL) {
            if (!decimal_point_entered && current_num_index < LCD_LINE_LEN - 1) { // Ensure space for '.' and at least one digit
                if (current_num_index == 0) { // If "." is the first char, prepend "0"
                    current_num_str[current_num_index++] = '0';
                }
                current_num_str[current_num_index++] = '.';
                current_num_str[current_num_index] = '\0';
                decimal_point_entered = true;
                last_key_was_operator = false;
            } else if (decimal_point_entered) {
                set_error("Err: Syntax"); // Multiple decimal points
            } else {
                set_error("Err: Num Len"); // Not enough space for decimal point
            }
        } else if (current_key >= KEY_PLUS && current_key <= KEY_DIVIDE) { // Operator keys
            char op_char_map[] = {'+', '-', '*', '/'}; // Map key codes to operator characters
            char selected_op_char = op_char_map[current_key - KEY_PLUS];

            // Handle unary minus: if '-' is pressed at start of expression,
            // or after another operator, and no number is currently being typed.
            if (selected_op_char == '-' && (expr_len == 0 || last_key_was_operator) && current_num_index == 0) {
                if (current_num_index < LCD_LINE_LEN) { // Check space for the '-' sign
                    current_num_str[current_num_index++] = '-';
                    current_num_str[current_num_index] = '\0';
                    last_key_was_operator = false; // Now considered part of number input
                } else {
                    set_error("Err: Num Len"); // Not enough space for '-'
                }
            } else { // Binary operator or non-unary context
                if (current_num_index > 0) { // If a number was being typed, process it
                    float num = parse_current_input_number();
                    if (!calculator_error) {
                        push_operand_to_expr(num);
                        add_to_expression_string(current_num_str); // Add parsed number to history
                    }
                    // Reset current number input state
                    current_num_index = 0;
                    memset(current_num_str, 0, sizeof(current_num_str));
                    decimal_point_entered = false;
                } else if (expr_len == 0 || (expr_type[expr_len - 1] == 'O' && !last_key_was_operator)) {
                    // Operator pressed without a preceding operand (e.g. "*5" or "5++")
                    // Allow leading '+' (often ignored) but error for other operators like '*' or '/'.
                    if (selected_op_char != '+') { 
                        set_error("Err: Syntax");
                    }
                }
                // If no error so far, push the operator
                if (!calculator_error) {
                    push_operator_to_expr(selected_op_char);
                    last_key_was_operator = true;
                }
            }
        } else if (current_key == KEY_EQUALS) { // Equals key
            // If a number is currently being typed, parse and push it
            if (current_num_index > 0 && !calculator_error) {
                float num = parse_current_input_number();
                if (!calculator_error) {
                    push_operand_to_expr(num);
                    add_to_expression_string(current_num_str); // Add final number to history
                }
            }
            
            // Check for syntax error: trailing operator (e.g., "5 + =")
            if (current_num_index == 0 && expr_len > 0 && expr_type[expr_len - 1] == 'O' && !calculator_error) {
                 set_error("Err: Syntax");
            }

            float final_result = 0.0f;
            if (!calculator_error) { // Only evaluate if no errors occurred during input phase
                final_result = evaluate_full_expression();
            }
            
            // Display result or error
            lcdchar(LCD_CMD_CLEAR_DISPLAY, 'C'); 
            delay(20); 

            if (calculator_error) { // If any error (input or evaluation)
                lcdstring(error_message);
            } else { // Display formatted result
                char result_str_buf[LCD_LINE_LEN + 1];
                
                // Check if the result is effectively an integer for display
                if (fabsf(final_result - roundf(final_result)) < FLOAT_EPSILON) {
                    snprintf(result_str_buf, sizeof(result_str_buf), "%.0f", final_result);
                } else {
                    // Format as float, then trim trailing zeros and unnecessary decimal point
                    snprintf(result_str_buf, sizeof(result_str_buf), "%f", final_result);
                    char *p = strchr(result_str_buf, '.');
                    if (p != NULL) {
                        int len = strlen(result_str_buf);
                        char *end_ptr = result_str_buf + len - 1;
                        while (end_ptr > p && *end_ptr == '0') {
                            *end_ptr-- = '\0'; // Remove trailing zero
                        }
                        if (*end_ptr == '.') { // If decimal point is now last char, remove it
                            *end_ptr = '\0'; 
                        }
                    }
                }

                // If formatted string is too long, try scientific notation
                if (strlen(result_str_buf) > LCD_LINE_LEN) {
                    snprintf(result_str_buf, sizeof(result_str_buf), "%.3e", final_result); // E.g., 3 decimal places for exponent
                }

                // If still too long, set display error
                if (strlen(result_str_buf) > LCD_LINE_LEN) { 
                    set_error("Err: Display"); 
                    lcdstring(error_message); // Display the new "Err: Display"
                } else {
                    lcdstring(result_str_buf);
                }
            }
            lcdchar(LCD_CMD_CURSOR_LINE_2, 'C'); // Move to second line
            delay(5); 
            lcdstring(""); // Clear the second line

            calculation_has_ended = true; // Set flag to indicate result/error is shown
            // Reset current number input state for the next potential calculation (after clear)
            current_num_index = 0; 
            memset(current_num_str, 0, sizeof(current_num_str));
            decimal_point_entered = false; 
            // expression_str and expr_len will be cleared by clear_all_state() when a new calculation starts.
        }
        
        // After processing a key, if not showing a final result/error, update the input display
        if (!calculation_has_ended) { 
             if (calculator_error) { // If an error was set during this key's processing (e.g. Num Len)
                lcdchar(LCD_CMD_CLEAR_DISPLAY, 'C'); 
                delay(20); 
                lcdstring(error_message);
                lcdchar(LCD_CMD_CURSOR_LINE_2, 'C'); 
                delay(5); 
                lcdstring("");
                calculation_has_ended = true; // Transition to error display state
             } else {
                update_lcd_display_content(); // Normal update of input display
             }
        }
        delay(100); // Debounce/polling delay at the end of the loop
    }
}

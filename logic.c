// ============= LOGIC.C =============
#include "logic.h"
#include "keypad.h"
#include "delay.h"
#include "lcd.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#define MAX_DISPLAY 32

char expr_type[MAX_TOKENS];
float expr_data[MAX_TOKENS];
int expr_len = 0;

char expression_str[MAX_DISPLAY];
int expression_index = 0;

// Current number being input
char current_num_str[16] = {0};
int current_num_index = 0;

void update_lcd_display() {
    char buffer[17];
    int start;
    int i;
    
    // Clear LCD first
    lcdchar(0x01, 'C');
    delay(20); // Increased delay after clear
    
    // Initialize buffer with null terminators
    for (i = 0; i < 17; i++) {
        buffer[i] = 0;
    }
    
    // Show expression on first line (last 16 chars)
    if (expression_index > 0) {
        start = expression_index >= 16 ? expression_index - 16 : 0;
        for (i = 0; i < 16 && (start + i) < expression_index; i++) {
            buffer[i] = expression_str[start + i];
        }
        buffer[i] = '\0'; // Ensure null termination
        lcdstring(buffer);
    }
    
    // Move to second line for current number
    lcdchar(0xC0, 'C'); // Move to second line
    delay(5);
    
    if (current_num_index > 0) {
        lcdstring(current_num_str);
    } else {
        lcdstring(" ");
    }
}

void add_to_expression(char* str) {
    int len = strlen(str);
    if (expression_index + len < MAX_DISPLAY - 1) { // Leave space for null terminator
        strcpy(&expression_str[expression_index], str);
        expression_index += len;
        expression_str[expression_index] = '\0'; // Ensure null termination
    }
}

void push_operand(float num) {
    if (expr_len >= MAX_TOKENS) return;
    expr_type[expr_len] = 'N';
    expr_data[expr_len++] = num;
}

void push_operator(char op) {
    char op_str[2];
    
    if (expr_len >= MAX_TOKENS) return;
    expr_type[expr_len] = 'O'; 
    expr_data[expr_len++] = op;
    
    // Add to display
    op_str[0] = op;
    op_str[1] = '\0';
    add_to_expression(op_str);
}

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

float apply_operator(char op, float a, float b) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? a / b : 9999999999999999; // Prevent divide by zero
        default: return 0;
    }
}

float evaluate_expression() {
    float val_stack[MAX_TOKENS];
    char op_stack[MAX_TOKENS];
    int val_top, op_top;
    int i;
    
    if (expr_len == 0) return 0;
    if (expr_len == 1 && expr_type[0] == 'N') return expr_data[0];
    
    val_top = -1;
    op_top = -1;

    for (i = 0; i < expr_len; i++) {
        if (expr_type[i] == 'N') {
            val_stack[++val_top] = expr_data[i];
        } else {
            char curr_op = (char)expr_data[i];
            while (op_top >= 0 && precedence(op_stack[op_top]) >= precedence(curr_op)) {
                char op;
                float b, a;
                if (val_top < 1) break; // Safety check
                op = op_stack[op_top--];
                b = val_stack[val_top--];
                a = val_stack[val_top--];
                val_stack[++val_top] = apply_operator(op, a, b);
            }
            op_stack[++op_top] = curr_op;
        }
    }

    while (op_top >= 0 && val_top >= 1) {
        char op = op_stack[op_top--];
        float b = val_stack[val_top--];  
        float a = val_stack[val_top--];
        val_stack[++val_top] = apply_operator(op, a, b);
    }

    return (val_top >= 0) ? val_stack[val_top] : 0;
}

void clear_all() {
    expr_len = 0;
    expression_index = 0;
    current_num_index = 0;
    memset(expression_str, 0, sizeof(expression_str));
    memset(current_num_str, 0, sizeof(current_num_str));
}

float parse_current_number() {
    float result;
    float decimal;
    bool is_decimal;
    bool is_negative;
    int start;
    int i;
    
    if (current_num_index == 0) return 0;
    
    // Simple string to float conversion
    result = 0;
    decimal = 0.1;
    is_decimal = false;
    is_negative = false;
    start = 0;
    
    if (current_num_str[0] == '-') {
        is_negative = true;
        start = 1;
    }
    
    for (i = start; i < current_num_index; i++) {
        if (current_num_str[i] == '.') {
            is_decimal = true;
        } else if (current_num_str[i] >= '0' && current_num_str[i] <= '9') {
            int digit = current_num_str[i] - '0';
            if (!is_decimal) {
                result = result * 10 + digit;
            } else {
                result += digit * decimal;
                decimal *= 0.1;
            }
        }
    }
    
    return is_negative ? -result : result;
}

void RunCalculatorLogic(void) {
    bool calculation_done = false;
    bool decimal_entered = false;
    bool operator_entered = false;
    unsigned char key = GetKeyPressed();
    clear_all();
    
    // Show calculator ready message
    lcdchar(0x01, 'C'); // Clear display
    delay(20);
    lcdstring("Calculator Ready");
    lcdchar(0xC0, 'C'); // Second line
    delay(5);
    lcdstring("Enter expression");
	while(key != 0xFF)
    update_lcd_display();

    while (!calculation_done) {
        key = GetKeyPressed();
        
        // Skip if no key pressed
        if (key == 0xFF) {
            delay(50);
            continue;
        }

        // Number keys (1-9)
        if (key >= 0x1 && key <= 0x9) {
            if (current_num_index < 15) {
                current_num_str[current_num_index++] = '0' + key;
                current_num_str[current_num_index] = '\0';
                operator_entered = false;
                update_lcd_display();
                
                // Wait for key release to prevent multiple entries
                while (GetKeyPressed() != 0xFF) {
                    delay(50);
                }
                delay(100); // Additional debounce delay
            }
        }
        // Zero key  
        else if (key == 0x0) {
            if (current_num_index < 15) {
                if (current_num_index == 0) {
                    // Allow leading zero
                    current_num_str[current_num_index++] = '0';
                } else {
                    current_num_str[current_num_index++] = '0';
                }
                current_num_str[current_num_index] = '\0';
                operator_entered = false;
                update_lcd_display();
                
                // Wait for key release
                while (GetKeyPressed() != 0xFF) {
                    delay(50);
                }
                delay(100);
            }
        }
        // Decimal point
        else if (key == 0xF) {
            if (!decimal_entered && current_num_index < 14) {
                if (current_num_index == 0) {
                    current_num_str[current_num_index++] = '0';
                }
                current_num_str[current_num_index++] = '.';
                current_num_str[current_num_index] = '\0';
                decimal_entered = true;
                update_lcd_display();
                
                // Wait for key release
                while (GetKeyPressed() != 0xFF) {
                    delay(50);
                }
                delay(100);
            }
        }
        // Operators (+, -, *, /)
        else if (key >= 0xA && key <= 0xD) {
            char op = 0;
            switch (key) {
                case 0xA: op = '+'; break;
                case 0xB: op = '-'; break;
                case 0xC: op = '*'; break;
                case 0xD: op = '/'; break;
            }

            // Handle negative numbers at start or after operator
            if (op == '-' && (expr_len == 0 || operator_entered) && current_num_index == 0) {
                current_num_str[current_num_index++] = '-';
                current_num_str[current_num_index] = '\0';
                update_lcd_display();
                
                // Wait for key release
                while (GetKeyPressed() != 0xFF) {
                    delay(50);
                }
                delay(100);
                continue;
            }

            // Process current number if exists
            if (current_num_index > 0) {
                float num = parse_current_number();
                push_operand(num);
                add_to_expression(current_num_str);
                
                // Reset current number
                current_num_index = 0;
                memset(current_num_str, 0, sizeof(current_num_str));
                decimal_entered = false;
            }
            
            // Add operator
            push_operator(op);
            operator_entered = true;
            update_lcd_display();
            
            // Wait for key release
            while (GetKeyPressed() != 0xFF) {
                delay(50);
            }
            delay(100);
        }
        // Equals
        else if (key == 0xE) {
            float result;
            char result_str[16];
            
            // Add final number if exists
            if (current_num_index > 0) {
                float num = parse_current_number();
                push_operand(num);
            }

            // Calculate result
            result = evaluate_expression();
            
            // Display result
            lcdchar(0x01, 'C'); // Clear display
            delay(20);
            lcdstring("Result:");
            lcdchar(0xC0, 'C'); // Second line
            delay(5);
            
            // Format result with proper precision
            if (result == (int)result) {
                sprintf(result_str, "%.0f", result); // No decimal for whole numbers
            } else {
                sprintf(result_str, "%.4g", result); // Up to 4 significant digits
            }
            lcdstring(result_str);
			delay(500000);
            
            calculation_done = true;
        }
        
        // Small delay to prevent excessive polling
        delay(20);
    }
}

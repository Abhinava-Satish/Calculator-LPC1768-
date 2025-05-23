#include <stdio.h>
#include <string.h> // For strcmp, strcpy, strlen
#include <math.h>   // For fabsf
#include "logic.h"  // The header for the code we are testing

// --- Global variables from logic.c needed by tests ---
// These are 'extern' in logic.h, so we need to define them here for the test executable.
bool calculator_error;
char error_message[ERROR_MSG_LEN];
char expr_type[MAX_TOKENS];
float expr_data[MAX_TOKENS];
int expr_len;
char expression_str[MAX_DISPLAY_STR];
int expression_index;
char current_num_str[LCD_LINE_LEN + 1];
int current_num_index;

// --- Assertion Macros ---
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_SETUP() do { \
    clear_all_state(); \
} while (0)

#define RUN_TEST(test_function) do { \
    printf("Running %s...\n", #test_function); \
    test_function(); \
} while (0)

#define ASSERT_TRUE(condition, message_format, ...) do { \
    if (!(condition)) { \
        printf(ANSI_COLOR_RED "[FAIL] " ANSI_COLOR_RESET); \
        printf(message_format, ##__VA_ARGS__); \
        printf(" (%s:%d)\n", __FILE__, __LINE__); \
        tests_failed++; \
    } else { \
        printf(ANSI_COLOR_GREEN "[PASS] " ANSI_COLOR_RESET); \
        printf(message_format, ##__VA_ARGS__); \
        printf("\n"); \
        tests_passed++; \
    } \
} while (0)

#define ASSERT_EQUAL_FLOAT(expected, actual, tolerance, message_format, ...) do { \
    if (fabsf((expected) - (actual)) > tolerance) { \
        printf(ANSI_COLOR_RED "[FAIL] " ANSI_COLOR_RESET); \
        printf(message_format, ##__VA_ARGS__); \
        printf(" - Expected %f, got %f (%s:%d)\n", expected, actual, __FILE__, __LINE__); \
        tests_failed++; \
    } else { \
        printf(ANSI_COLOR_GREEN "[PASS] " ANSI_COLOR_RESET); \
        printf(message_format, ##__VA_ARGS__); \
        printf("\n"); \
        tests_passed++; \
    } \
} while (0)

#define ASSERT_EQUAL_STRING(expected, actual, message_format, ...) do { \
    if (strcmp(expected, actual) != 0) { \
        printf(ANSI_COLOR_RED "[FAIL] " ANSI_COLOR_RESET); \
        printf(message_format, ##__VA_ARGS__); \
        printf(" - Expected '%s', got '%s' (%s:%d)\n", expected, actual, __FILE__, __LINE__); \
        tests_failed++; \
    } else { \
        printf(ANSI_COLOR_GREEN "[PASS] " ANSI_COLOR_RESET); \
        printf(message_format, ##__VA_ARGS__); \
        printf("\n"); \
        tests_passed++; \
    } \
} while (0)


// --- Test Cases for parse_current_input_number ---

void test_parse_integer() {
    TEST_SETUP();
    strcpy(current_num_str, "123");
    current_num_index = strlen(current_num_str);
    float result = parse_current_input_number();
    ASSERT_EQUAL_FLOAT(123.0f, result, 1e-6f, "Parse integer '123'");
    ASSERT_TRUE(!calculator_error, "No error for '123'");
}

void test_parse_zero() {
    TEST_SETUP();
    strcpy(current_num_str, "0");
    current_num_index = strlen(current_num_str);
    float result = parse_current_input_number();
    ASSERT_EQUAL_FLOAT(0.0f, result, 1e-6f, "Parse integer '0'");
    ASSERT_TRUE(!calculator_error, "No error for '0'");
}

void test_parse_float() {
    TEST_SETUP();
    strcpy(current_num_str, "12.34");
    current_num_index = strlen(current_num_str);
    float result = parse_current_input_number();
    ASSERT_EQUAL_FLOAT(12.34f, result, 1e-6f, "Parse float '12.34'");
    ASSERT_TRUE(!calculator_error, "No error for '12.34'");
}

void test_parse_float_leading_dot() { // Input logic should make this "0.5"
    TEST_SETUP();
    strcpy(current_num_str, "0.5"); // Simulating input logic of ".5" becoming "0.5"
    current_num_index = strlen(current_num_str);
    float result = parse_current_input_number();
    ASSERT_EQUAL_FLOAT(0.5f, result, 1e-6f, "Parse float '0.5' (from .5)");
    ASSERT_TRUE(!calculator_error, "No error for '0.5'");
}

void test_parse_negative_integer() {
    TEST_SETUP();
    strcpy(current_num_str, "-10");
    current_num_index = strlen(current_num_str);
    float result = parse_current_input_number();
    ASSERT_EQUAL_FLOAT(-10.0f, result, 1e-6f, "Parse negative integer '-10'");
    ASSERT_TRUE(!calculator_error, "No error for '-10'");
}

void test_parse_negative_float() {
    TEST_SETUP();
    strcpy(current_num_str, "-3.14");
    current_num_index = strlen(current_num_str);
    float result = parse_current_input_number();
    ASSERT_EQUAL_FLOAT(-3.14f, result, 1e-6f, "Parse negative float '-3.14'");
    ASSERT_TRUE(!calculator_error, "No error for '-3.14'");
}

void test_parse_error_multiple_decimals() {
    TEST_SETUP();
    strcpy(current_num_str, "1.2.3");
    current_num_index = strlen(current_num_str);
    parse_current_input_number(); // Result doesn't matter, check error state
    ASSERT_TRUE(calculator_error, "Error flag for '1.2.3'");
    ASSERT_EQUAL_STRING("Err: Syntax", error_message, "Error message for '1.2.3'");
}

void test_parse_error_invalid_chars() {
    TEST_SETUP();
    strcpy(current_num_str, "12a3");
    current_num_index = strlen(current_num_str);
    parse_current_input_number();
    ASSERT_TRUE(calculator_error, "Error flag for '12a3'");
    ASSERT_EQUAL_STRING("Err: Syntax", error_message, "Error message for '12a3'");
}

void test_parse_empty_string() {
    TEST_SETUP();
    current_num_str[0] = '\0';
    current_num_index = 0;
    float result = parse_current_input_number();
    ASSERT_EQUAL_FLOAT(0.0f, result, 1e-6f, "Parse empty string");
    ASSERT_TRUE(!calculator_error, "No error for empty string");
}

void test_parse_standalone_minus() {
    TEST_SETUP();
    strcpy(current_num_str, "-");
    current_num_index = 1;
    parse_current_input_number();
    ASSERT_TRUE(calculator_error, "Error flag for '-'");
    ASSERT_EQUAL_STRING("Err: Syntax", error_message, "Error message for '-'");
}

// --- Helper function for evaluate_full_expression tests ---
void setup_expression(const char types[], const float data[], int length) {
    TEST_SETUP(); // Includes clear_all_state()
    expr_len = length;
    for (int i = 0; i < length; ++i) {
        expr_type[i] = types[i];
        expr_data[i] = data[i];
    }
}

// --- Test Cases for evaluate_full_expression ---

void test_eval_addition() {
    char types[] = {'N', 'O', 'N'};
    float data[] = {2.0f, (float)'+', 3.0f};
    setup_expression(types, data, 3);
    float result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(5.0f, result, 1e-6f, "Eval: 2+3");
    ASSERT_TRUE(!calculator_error, "Eval: 2+3 no error");
}

void test_eval_subtraction() {
    char types[] = {'N', 'O', 'N'};
    float data[] = {5.0f, (float)'-', 2.0f};
    setup_expression(types, data, 3);
    float result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(3.0f, result, 1e-6f, "Eval: 5-2");
    ASSERT_TRUE(!calculator_error, "Eval: 5-2 no error");
}

void test_eval_multiplication() {
    char types[] = {'N', 'O', 'N'};
    float data[] = {3.0f, (float)'*', 4.0f};
    setup_expression(types, data, 3);
    float result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(12.0f, result, 1e-6f, "Eval: 3*4");
    ASSERT_TRUE(!calculator_error, "Eval: 3*4 no error");
}

void test_eval_division() {
    char types[] = {'N', 'O', 'N'};
    float data[] = {10.0f, (float)'/', 2.0f};
    setup_expression(types, data, 3);
    float result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(5.0f, result, 1e-6f, "Eval: 10/2");
    ASSERT_TRUE(!calculator_error, "Eval: 10/2 no error");
}

void test_eval_precedence() {
    // 2+3*4 = 14
    char types[] = {'N', 'O', 'N', 'O', 'N'};
    float data[] = {2.0f, (float)'+', 3.0f, (float)'*', 4.0f};
    setup_expression(types, data, 5);
    float result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(14.0f, result, 1e-6f, "Eval: 2+3*4");
    ASSERT_TRUE(!calculator_error, "Eval: 2+3*4 no error");
}

void test_eval_precedence_2() {
    // 2*3+4 = 10
    char types[] = {'N', 'O', 'N', 'O', 'N'};
    float data[] = {2.0f, (float)'*', 3.0f, (float)'+', 4.0f};
    setup_expression(types, data, 5);
    float result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(10.0f, result, 1e-6f, "Eval: 2*3+4");
    ASSERT_TRUE(!calculator_error, "Eval: 2*3+4 no error");
}


void test_eval_same_precedence_ltr() {
    // 10-2+3 = 11
    char types[] = {'N', 'O', 'N', 'O', 'N'};
    float data[] = {10.0f, (float)'-', 2.0f, (float)'+', 3.0f};
    setup_expression(types, data, 5);
    float result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(11.0f, result, 1e-6f, "Eval: 10-2+3");
    ASSERT_TRUE(!calculator_error, "Eval: 10-2+3 no error");
}

void test_eval_float_result() {
    // 1/2 = 0.5
    char types[] = {'N', 'O', 'N'};
    float data[] = {1.0f, (float)'/', 2.0f};
    setup_expression(types, data, 3);
    float result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(0.5f, result, 1e-6f, "Eval: 1/2");
    ASSERT_TRUE(!calculator_error, "Eval: 1/2 no error");
}

void test_eval_negative_numbers() {
    // -2+5 = 3
    char types[] = {'N', 'O', 'N'};
    float data[] = {-2.0f, (float)'+', 5.0f};
    setup_expression(types, data, 3);
    float result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(3.0f, result, 1e-6f, "Eval: -2+5");
    ASSERT_TRUE(!calculator_error, "Eval: -2+5 no error");

    // 5*-2 = -10
    char types2[] = {'N', 'O', 'N'};
    float data2[] = {5.0f, (float)'*', -2.0f};
    setup_expression(types2, data2, 3);
    result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(-10.0f, result, 1e-6f, "Eval: 5*-2");
    ASSERT_TRUE(!calculator_error, "Eval: 5*-2 no error");
}

void test_eval_division_by_zero() {
    char types[] = {'N', 'O', 'N'};
    float data[] = {1.0f, (float)'/', 0.0f};
    setup_expression(types, data, 3);
    evaluate_full_expression(); // Result doesn't matter
    ASSERT_TRUE(calculator_error, "Eval: 1/0 error flag");
    ASSERT_EQUAL_STRING("Err: Div Zero", error_message, "Eval: 1/0 error message");
}

void test_eval_error_syntax_trailing_operator() {
    // "5*+"
    char types[] = {'N', 'O', 'O'};
    float data[] = {5.0f, (float)'*', (float)'+'};
    setup_expression(types, data, 3);
    evaluate_full_expression();
    ASSERT_TRUE(calculator_error, "Eval: 5*+ error flag");
    ASSERT_EQUAL_STRING("Err: Syntax", error_message, "Eval: 5*+ error message");
}

void test_eval_single_number() {
    char types[] = {'N'};
    float data[] = {7.0f};
    setup_expression(types, data, 1);
    float result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(7.0f, result, 1e-6f, "Eval: 7");
    ASSERT_TRUE(!calculator_error, "Eval: 7 no error");
}

void test_eval_error_expr_long_push() {
    TEST_SETUP();
    // Try to push more than MAX_TOKENS
    for(int i=0; i < MAX_TOKENS + 1; ++i) {
        push_operand_to_expr((float)i); // This will eventually fail
        if (calculator_error) break;
        if (i < MAX_TOKENS) { // Avoid pushing operator after last operand if already too long
             push_operator_to_expr('+');
             if (calculator_error) break;
        }
    }
    ASSERT_TRUE(calculator_error, "Eval: Expression too long (via push) error flag");
    ASSERT_EQUAL_STRING("Err: Expr Long", error_message, "Eval: Expression too long (via push) error message");
}

void test_eval_empty_expression() {
    TEST_SETUP();
    expr_len = 0;
    float result = evaluate_full_expression();
    ASSERT_EQUAL_FLOAT(0.0f, result, 1e-6f, "Eval: Empty expression");
    ASSERT_TRUE(!calculator_error, "Eval: Empty expression no error");
}


// --- Main Test Runner ---
int main() {
    printf("Starting unit tests for logic.c...\n\n");

    printf("--- Testing parse_current_input_number ---\n");
    RUN_TEST(test_parse_integer);
    RUN_TEST(test_parse_zero);
    RUN_TEST(test_parse_float);
    RUN_TEST(test_parse_float_leading_dot);
    RUN_TEST(test_parse_negative_integer);
    RUN_TEST(test_parse_negative_float);
    RUN_TEST(test_parse_error_multiple_decimals);
    RUN_TEST(test_parse_error_invalid_chars);
    RUN_TEST(test_parse_empty_string);
    RUN_TEST(test_parse_standalone_minus);
    printf("\n");

    printf("--- Testing evaluate_full_expression ---\n");
    RUN_TEST(test_eval_addition);
    RUN_TEST(test_eval_subtraction);
    RUN_TEST(test_eval_multiplication);
    RUN_TEST(test_eval_division);
    RUN_TEST(test_eval_precedence);
    RUN_TEST(test_eval_precedence_2);
    RUN_TEST(test_eval_same_precedence_ltr);
    RUN_TEST(test_eval_float_result);
    RUN_TEST(test_eval_negative_numbers);
    RUN_TEST(test_eval_division_by_zero);
    RUN_TEST(test_eval_error_syntax_trailing_operator);
    RUN_TEST(test_eval_single_number);
    RUN_TEST(test_eval_error_expr_long_push); // Tests push functions setting error
    RUN_TEST(test_eval_empty_expression);


    printf("\n--- Test Summary ---\n");
    printf(ANSI_COLOR_GREEN "Passed: %d\n" ANSI_COLOR_RESET, tests_passed);
    printf(ANSI_COLOR_RED "Failed: %d\n" ANSI_COLOR_RESET, tests_failed);
    printf("\n");

    return (tests_failed == 0) ? 0 : 1;
}

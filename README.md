# Embedded Calculator for LPC1768

A simple yet robust calculator application designed for the LPC1768 microcontroller. It supports floating-point arithmetic, evaluation of multi-operation expressions, and provides user-friendly feedback through an LCD display.

## Features & Improvements

This calculator has undergone significant enhancements to improve its functionality, reliability, and maintainability:

*   **Expression Evaluation**: Supports evaluating expressions with multiple arithmetic operations (+, -, \*, /) respecting standard operator precedence.
*   **Floating-Point Arithmetic**: Handles floating-point numbers for both input and calculations.
*   **Refined Input Handling**: User input from the keypad is processed for better responsiveness and a smoother user experience.
*   **Comprehensive Error Handling**: The calculator can detect and display various error conditions, including:
    *   `Err: Div Zero` (Division by zero)
    *   `Err: Syntax` (Invalid expression structure or input sequence)
    *   `Err: Expr Long` (Expression exceeds the maximum number of tokens)
    *   `Err: Num Len` (Input number is too long for the display or internal buffers)
    *   `Err: Stack` (Internal error during expression evaluation, e.g., stack overflow)
    *   `Err: Display` (Resulting number is too large or too small to be displayed correctly)
*   **Improved Floating-Point Display**: Calculation results are displayed with enhanced precision. Integers are shown without trailing decimal points/zeros. Floating-point numbers are formatted to fit the display, removing unnecessary trailing zeros, and using scientific notation if the number is too long.
*   **Unit Tests**: Core calculation logic (`logic.c`) is supported by a suite of unit tests to verify parsing and evaluation correctness.
*   **Code Quality**: The codebase has been cleaned up with consistent formatting and extensive comments for better readability and maintainability. Key constants are well-defined.

## Building and Running Unit Tests

Unit tests for the core logic module (`logic.c`) are provided in `test_logic.c` and can be compiled and run in a standard C environment (e.g., using GCC). These tests verify the `parse_current_input_number` and `evaluate_full_expression` functions.

To compile and run the unit tests:

1.  Ensure you have GCC (or a compatible C compiler) installed.
2.  Navigate to the project directory containing all source files (`logic.c`, `logic.h`, `test_logic.c`, `test_stubs.c`, `LPC17xx.h` (dummy), `keypad.h`, `lcd.h`, `delay.h`).
3.  Compile the test suite using the following command:
    ```bash
    gcc -o test_logic logic.c test_stubs.c test_logic.c -lm -std=c99
    ```
4.  Execute the compiled tests:
    ```bash
    ./test_logic
    ```
    The test runner will output the status of each test and a final summary.

## Known Limitations & Assumptions

*   **Target Hardware**: The project is specifically designed for the NXP LPC1768 microcontroller. It assumes the presence of a compatible 4x4 keypad and a character LCD (interfaced as per `keypad.c` and `lcd.c`).
*   **Software Delays**: Timing and delays (e.g., for LCD interaction, debouncing) are implemented using software-based busy-wait loops (`delay.c`). These are sensitive to the microcontroller's clock speed and may require adjustment if the clock configuration differs from the one assumed during development.
*   **Simulated Unit Tests**: The provided unit tests run in a simulated (host) environment, not on the target LPC1768 hardware. While they validate the core logic, they do not cover hardware interactions or real-time behavior.
*   **Expression Length**: While `MAX_TOKENS` (default 50) allows for complex expressions, the `MAX_DISPLAY_STR` (default 32) limits the length of the expression history shown on the LCD's first line. Extremely long numbers or many short numbers/operators might not fully display in the history.
*   **Floating Point Precision**: Uses standard `float` type, which has inherent precision limitations.

This README provides a more accurate overview of the calculator project's current capabilities and context.

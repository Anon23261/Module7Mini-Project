#include <iostream>
#include <string>

// Tutorial 1: Introduction to C++
// This tutorial covers basic C++ syntax, variables, and input/output

int main() {
    // Print a welcome message
    std::cout << "Welcome to C++ Programming!\n\n";

    // Variables and data types
    int number = 42;
    double pi = 3.14159;
    std::string name;

    // User input
    std::cout << "What's your name? ";
    std::getline(std::cin, name);

    // String concatenation and output formatting
    std::cout << "\nHello, " << name << "!\n";
    std::cout << "Here are some numbers:\n";
    std::cout << "Integer: " << number << "\n";
    std::cout << "Double: " << pi << "\n";

    // Basic arithmetic
    int a = 10, b = 3;
    std::cout << "\nArithmetic Operations:\n";
    std::cout << a << " + " << b << " = " << (a + b) << "\n";
    std::cout << a << " - " << b << " = " << (a - b) << "\n";
    std::cout << a << " * " << b << " = " << (a * b) << "\n";
    std::cout << a << " / " << b << " = " << (a / b) << " (integer division)\n";
    std::cout << a << " % " << b << " = " << (a % b) << " (modulus)\n";

    // Type casting
    std::cout << a << " / " << b << " = " << (static_cast<double>(a) / b) << " (floating-point division)\n";

    // Constants
    const int DAYS_IN_WEEK = 7;
    std::cout << "\nThere are " << DAYS_IN_WEEK << " days in a week.\n";

    // Basic control flow
    std::cout << "\nEnter a number: ";
    int user_number;
    std::cin >> user_number;

    if (user_number > 0) {
        std::cout << "You entered a positive number.\n";
    } else if (user_number < 0) {
        std::cout << "You entered a negative number.\n";
    } else {
        std::cout << "You entered zero.\n";
    }

    // Exercise for the reader:
    std::cout << "\nExercise:\n";
    std::cout << "1. Create variables of different types (int, double, string)\n";
    std::cout << "2. Perform arithmetic operations with them\n";
    std::cout << "3. Use cout to display the results\n";

    return 0;
}

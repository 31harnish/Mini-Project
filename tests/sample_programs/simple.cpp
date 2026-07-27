#include <iostream>

// Simple arithmetic function (low complexity)
int add_numbers(int a, int b) {
    return a + b;
}

// Simple control flow (2 basic blocks)
bool is_even(int number) {
    if (number % 2 == 0) {
        return true;
    }
    return false;
}

int main() {
    int x = 5;
    int y = 10;
    std::cout << "Sum: " << add_numbers(x, y) << std::endl;
    std::cout << "Is " << x << " even? " << (is_even(x) ? "Yes" : "No") << std::endl;
    return 0;
}

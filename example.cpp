// Example usage of Linear Hashing with random numbers
#include "linear_hashing.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

int main() {
    // Instantiate Linear Hashing with default parameters
    Linear_Hashing<int> lh;

    // Seed for random number generation
    std::srand(std::time(0));

    // Insert random numbers up to 100
    for (int i = 0; i < 100; ++i) {
        int random_number = std::rand() % 100 + 1;  // Generate a random number between 1 and 100
        lh.insert(random_number);
    }

    // Iterate over elements(may not be stored in order they were inserted)
    for (const auto& element : lh) {
        std::cout << element << " ";
    }

    return 0;
}

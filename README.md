# Linear Hashing (LH) Implementation in C++

## Overview

This C++ project provides a dynamic and memory-efficient implementation of Linear Hashing (LH), designed specifically to be integrated into larger projects. The implementation is crafted without the use of Standard Template Library (STL) containers, opting for dynamic arrays, pointers, and custom memory management for enhanced control and performance.

## Project Motivation

This Linear Hashing implementation is not only a standalone data structure but has been meticulously developed to be seamlessly integrated into larger projects. The primary motivation behind this project is to foster a deeper understanding of data structures, particularly in scenarios where control over memory management and efficient element manipulation is paramount.

## Key Features

- **Dynamic Arrays:** Utilizes dynamic arrays for efficient and flexible memory management.
- **Pointer Navigation:** Employs pointers for traversing LH data structures, optimizing element manipulation.
- **Custom Memory Management:** Implements a custom destructor for proper resource deallocation.
- **No STL Containers:** Avoids STL containers for a lightweight and tailored solution.

## Components

- **Bucket Structure:** LH structure comprises buckets, each containing an array of elements.
- **Hashing Strategies:** Employs hashing strategies for uniform element placement within the LH structure.
- **Splitting and Rehashing:** Dynamically adjusts size through splitting and rehashing, adapting to changing workloads.

## Usage

### Getting Started

1. Clone the repository:

    ```bash
    git clone https://github.com/DEnsM0/LH-Implementation.git
    ```

2. Build the project using your preferred C++ compiler.

### Example

```cpp
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


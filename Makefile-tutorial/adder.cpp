#include <iostream>
#include "adder.h"

int secret = 10;

Adder::Adder(int x) noexcept : secret_number(x) {
    std::cout << "Adder: I got a secret number" << std::endl;
}

int Adder::add(int y) noexcept {
    return secret_number + y;
}

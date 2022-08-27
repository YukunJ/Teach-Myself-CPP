#include <iostream>
#include "adder.h"

using namespace std;

int main() {
    Adder a(secret);
    int input, result, guess;
    cout << "Enter a number please: ";
    cin >> input;
    result = a.add(input);
    guess = result - input;
    cout << "I guess the secret key is " << guess << endl;
    return 0;
}

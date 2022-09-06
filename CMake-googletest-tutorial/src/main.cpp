#include <iostream>
#include "math.h"

int main(int argc, const char* argv[]) {
    int result;
    
    result = math::add(1, 2);
    std::cout << "math::add(1, 2) = " << result << std::endl;
  
    result = math::sub(4, 2);
    std::cout << "math::sub(4, 2) = " << result << std::endl;
    
    result = math::mul(3, 2);
    std::cout << "math::mul(3, 2) = " << result << std::endl;
    
    return 0;
}

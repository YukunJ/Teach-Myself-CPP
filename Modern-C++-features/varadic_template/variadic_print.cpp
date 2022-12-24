#include <iostream>
#include <string>

template <typename T>
std::ostream& print(std::ostream & os, const T& t) {
    return os << t;
}

template<typename T, typename... Args>
std::ostream& print(std::ostream & os, const T& t, const Args&... rest) {
    os << t << ",";
    return print(os, rest...);
}

int main() {
    std::string greeting = "Yoo";
    print(std::cout, 1, 2.0, "hello", greeting);
    return 0;
}
#include <iostream>
#include <string>

template <typename T>
std::string debug_prefix(const T& t) {
   return "debug:" + std::to_string(t);
}

template<typename T>
std::ostream& print(std::ostream& os, const T& t) {
  return os << t;
}

template<typename T, typename... Args>
std::ostream& print(std::ostream& os, const T& t, const Args&... rest) {
  os << t << ",";
  return print(os, rest...);
}

template <typename... Args>
std::ostream& debug_print(std::ostream& os, const Args&... rest) {
    return print(os, debug_prefix(rest)...);
}

int main() {
    std::string greeting = "Yoo";
    debug_print(std::cout, 1, 2.2, 3.5);
    return 0;
}
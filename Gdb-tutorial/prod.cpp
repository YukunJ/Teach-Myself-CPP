// prod.cpp
#include <iostream>
#include <vector>

/**
 * @brief return the product of the input array
 * @param array the input array
 * @ret prod of the array, or 0 if it is empty
 */
int prod(std::vector<int> array) {
  if (array.empty()) {
    return 0;
  }
  int prod = 1;
  for (int i = 0; i <= array.size(); i++) { // <- problem here, the '<='
    prod *= array[i];
  }
  return prod;
}

int main() {
  std::vector<int> experiment {1, 2, 3, 4, 5};
  std::cout << "The prod of <1, 2, 3, 4, 5> is " << prod(experiment) << std::endl; // should be 120
}

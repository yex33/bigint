#include <iostream>

#include "bigint.hpp"

int main() {
  std::cout << "Hello world!" << std::endl;
  bool res = bigint(12345) == bigint(12345);
  std::cout << "should be 1: " << (int) res << std::endl;
}

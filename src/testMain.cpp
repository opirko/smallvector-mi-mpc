#include <iostream>

#include "smallVector.hpp"

int main(int, char**) {
  std::cout << "Test Main start." << std::endl;
  mpc::smallVector<int> vec(5);
  std::cout << "Test Main end." << std::endl;
  return 0;
}

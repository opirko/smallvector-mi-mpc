#include <iostream>

#include "src/smallVector.hpp"

int main(int argc, char** argv) {
  std::cout << "Test Main start." << std::endl;
  mpc::smallVector<int> vec(5);
  std::cout << "Test Main end." << std::endl;
  return 0;
}

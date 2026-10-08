#include "server/server.hpp"
#include <iostream>

int main() {
  Server server;

  try {
    server.start();
  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << '\n';

    return 1;
  }

  return 0;
}

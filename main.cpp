#include "http/http-response.hpp"
#include "router/router.hpp"
#include "server/server.hpp"
#include <iostream>

int main() {
  Server server;

  Router router;

  router.get("/api", []() {
    HttpResponse res(HttpStatus::OK);

    return res;
  });

  try {
    server.start(3000, router);
  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << '\n';

    return 1;
  }

  return 0;
}

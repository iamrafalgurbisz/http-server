#include "http/http-response.hpp"
#include "router/router.hpp"
#include "server/server.hpp"
#include <iostream>

int main() {
  Router router;

  router.get("/api", []() {
    HttpResponse res(HttpStatus::OK, "Hello from the server!");

    return res;
  });

  Server server(router);

  try {
    server.start(3000);
  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << '\n';

    return 1;
  }

  return 0;
}

#include "router/router.hpp"
class Server {
public:
  void start(int PORT, Router router);
  void close();

private:
  int server_socket_ = -1;
  int client_socket_ = -1;
};

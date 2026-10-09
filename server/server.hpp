#include "http/http-response.hpp"
#include "router/router.hpp"

class Server {
public:
  Server(Router router);
  void start(int PORT);
  void close();

private:
  int server_socket_ = -1;
  int client_socket_ = -1;
  Router router_;

  void loop_();
  void respond_(HttpResponse res);
};

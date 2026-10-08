class Server {
public:
  void start();
  void close();

private:
  int server_socket = -1;
  int client_socket = -1;
};

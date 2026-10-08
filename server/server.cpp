#include "server/server.hpp"
#include "http/http-request.hpp"
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

void Server::start() {
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    throw std::system_error(errno, std::generic_category(), "bind");
  }

  int opt = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(1234);
  server_address.sin_addr.s_addr = INADDR_ANY;

  int bind_res = bind(server_socket, (struct sockaddr *)&server_address,
                      sizeof(server_address));

  if (bind_res < 0) {
    throw std::system_error(errno, std::generic_category(), "bind");
  }

  int listener = listen(server_socket, 5);
  if (listener < 0) {
    throw std::system_error(errno, std::generic_category(), "bind");
  }

  std::cout << "Listening" << "\n";

  while (true) {

    client_socket = accept(server_socket, nullptr, nullptr);

    if (client_socket < 0) {
      throw std::system_error(errno, std::generic_category(), "accept");
    }

    std::string buffer;
    HttpRequest req;

    while (true) {
      char temp[4096];

      ssize_t n = recv(client_socket, temp, sizeof(temp), 0);

      std::cout << "received: " << n << " bytes\n";

      if (n == 0) {
        std::cout << "Client disconnected" << "\n";
        break;
      }

      if (n < 0) {
        std::perror("recv");
        break;
      }

      buffer.append(temp, n);

      HttpRequestParseResult parse_result = req.parse(buffer);

      if (parse_result == HttpRequestParseResult::Incomplete) {
        continue;
      }

      if (parse_result == HttpRequestParseResult::Invalid) {
        // TODO: send 400 Bad Request
        break;
      }

      // Complete
      std::cout << "----- Request, " << buffer.size() << " bytes -----\n";

      std::cout << "Method: " << req.method() << "\n";
      std::cout << "Version: " << req.version() << "\n";
      std::cout << "Host: " << req.host() << "\n";
      std::cout << "User-Agent: " << req.user_agent() << "\n";
      std::cout << "Accept: " << req.accept() << "\n";
      std::cout << "Content-Length: " << req.content_length() << "\n";
      std::cout << "Content-Type: " << req.content_type() << "\n";
      std::cout << "Body: " << req.body() << "\n";

      break;
    }

    ::close(client_socket);
  }
}

// not used, implement with SIGINT and running flag later
void Server::close() {
  if (client_socket >= 0) {
    ::close(client_socket);
  }

  if (server_socket >= 0) {
    ::close(server_socket);
  }
}

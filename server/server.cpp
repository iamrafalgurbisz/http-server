#include "server/server.hpp"
#include "http/http-request.hpp"
#include "http/http-response.hpp"
#include "router/router.hpp"
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(Router router) { router_ = router; }

void Server::start(int PORT) {
  server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket_ < 0) {
    throw std::system_error(errno, std::generic_category(), "bind");
  }

  int opt = 1;
  int sockopt =
      setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (sockopt < 0) {
    throw std::system_error(errno, std::generic_category(), "setsockopt");
  }

  sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);
  server_address.sin_addr.s_addr = INADDR_ANY;

  int bind_res = bind(server_socket_, (struct sockaddr *)&server_address,
                      sizeof(server_address));

  if (bind_res < 0) {
    throw std::system_error(errno, std::generic_category(), "bind");
  }

  int listener = listen(server_socket_, 5);
  if (listener < 0) {
    throw std::system_error(errno, std::generic_category(), "bind");
  }

  std::cout << "Listening on port " << PORT << "..." << "\n";

  loop_();
}

void Server::loop_() {
  while (true) {
    client_socket_ = accept(server_socket_, nullptr, nullptr);

    if (client_socket_ < 0) {
      throw std::system_error(errno, std::generic_category(), "accept");
    }

    std::string buffer;
    HttpRequest req;

    /*
     * ========================================
     * second loop because TCP is streamed,
     * and we need to make sure that we got the whole response
     * ========================================
     */
    while (true) {
      char temp[4096];

      ssize_t n = recv(client_socket_, temp, sizeof(temp), 0);

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
        HttpResponse res(HttpStatus::BAD_REQUEST);

        respond_(res);

        break;
      }

      std::cout << "[" << req.method().str() << "] " << req.path() << "\n";

      if (router_.handlers().find(req.method().str() + req.path()) !=
          router_.handlers().end()) {
        HttpResponse res =
            router_.handlers().find(req.method().str() + req.path())->second();

        respond_(res);

        break;
      } else {
        HttpResponse res(HttpStatus::NOT_FOUND);

        respond_(res);

        break;
      }
    }

    ::close(client_socket_);
  }
}

void Server::respond_(HttpResponse res) {
  size_t total_sent = 0;

  while (total_sent < res.response().size()) {
    ssize_t sent = send(client_socket_, res.response().data() + total_sent,
                        res.response().size() - total_sent, 0);

    if (sent < 0) {
      if (errno == EINTR) {
        continue;
      }

      throw std::system_error(errno, std::generic_category(), "send");
    }

    if (sent == 0) {
      throw std::runtime_error("send returned 0");
    }

    total_sent += static_cast<size_t>(sent);
  }
}

// not used, implement with SIGINT and running flag later
void Server::close() {
  if (client_socket_ >= 0) {
    ::close(client_socket_);
  }

  if (server_socket_ >= 0) {
    ::close(server_socket_);
  }
}

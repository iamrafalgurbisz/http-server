#include "server/server.hpp"
#include "http/http-request.hpp"
#include "http/http-response.hpp"
#include "router/router.hpp"
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

void Server::start(int PORT, Router router) {
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
      std::cout << "Path: " << req.path() << "\n";
      std::cout << "Version: " << req.version() << "\n";
      std::cout << "Host: " << req.host() << "\n";
      std::cout << "User-Agent: " << req.user_agent() << "\n";
      std::cout << "Accept: " << req.accept() << "\n";
      std::cout << "Content-Length: " << req.content_length() << "\n";
      std::cout << "Content-Type: " << req.content_type() << "\n";
      std::cout << "Body: " << req.body() << "\n";

      std::cout << "Look for handler " << req.path() << "\n";

      if (router.handlers().find(req.path()) != router.handlers().end()) {
        std::cout << "Handler found!" << "\n";

        HttpResponse res = router.handlers().find(req.path())->second();

        size_t total_sent = 0;

        while (total_sent < res.response().size()) {
          ssize_t sent =
              send(client_socket_, res.response().data() + total_sent,
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

        std::cout << "response" << res.response() << "\n";

        break;
      } else {
        break;
      }
    }

    ::close(client_socket_);
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

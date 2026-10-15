#include "server/server.hpp"
#include "http/http-method.hpp"
#include "http/http-request.hpp"
#include "http/http-response.hpp"
#include "router/router.hpp"
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

Server::Server(const Router &router) : router_(router) {};

void Server::start(int PORT) {
  constexpr std::size_t MAX_BODY_SIZE = 32 * 1024;

  server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket_ < 0) {
    throw std::system_error(errno, std::generic_category(), "server_socket");
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
    throw std::system_error(errno, std::generic_category(), "listen");
  }

  std::cout << "Listening on port " << PORT << "..." << "\n";

  loop_();
}

void Server::loop_() {
  while (true) {
    bool already_responded = false;
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

      std::cout << "[" << req.method().str() << "] " << req.path() << "\n";

      if (parse_result == HttpRequestParseResult::Incomplete) {
        continue;
      }

      if (parse_result == HttpRequestParseResult::UriTooLarge) {
        respond_(HttpResponse(HttpStatus::URI_TOO_LARGE));

        break;
      }

      if (parse_result == HttpRequestParseResult::HeadersTooLarge) {
        respond_(HttpResponse(HttpStatus::HEADERS_TOO_LARGE));

        break;
      }

      if (parse_result == HttpRequestParseResult::BodyTooLarge) {
        respond_(HttpResponse(HttpStatus::BODY_TOO_LARGE));

        break;
      }

      if (parse_result == HttpRequestParseResult::UnknownExpectation) {
        respond_(HttpResponse(HttpStatus::EXPECTATION_FAILED));

        break;
      }

      if (parse_result == HttpRequestParseResult::Invalid) {
        std::cout << "Respond with bad request\n";

        respond_(HttpResponse(HttpStatus::BAD_REQUEST));

        break;
      }

      if (req.expect() == Expectation::Continue) {
        std::cout << "waiting for body...\n";

        HttpResponse res(HttpStatus::CONTINUE);

        if (!already_responded) {
          respond_(res);
          already_responded = true;
        }

        if (parse_result == HttpRequestParseResult::Incomplete) {
          continue;
        }
      }

      if (router_.handlers().find(req.path()) != router_.handlers().end()) {
        const auto &path_handlers = router_.handlers().find(req.path())->second;

        if (req.method().key() == HttpMethodKey::Head &&
            path_handlers.find(HttpMethodKey::Get) != path_handlers.end()) {
          std::cout << "HEAD request " << req.path() << " was mapped to GET\n";

          HttpResponse res =
              path_handlers.find(HttpMethodKey::Get)->second(req);

          respond_(res);

          break;
        }

        if (path_handlers.find(req.method().key()) == path_handlers.end()) {
          HttpResponse res(HttpStatus::METHOD_NOT_ALLOWED);

          respond_(res);

          break;
        }

        HttpResponse res = path_handlers.find(req.method().key())->second(req);

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

void Server::respond_(const HttpResponse &res) {
  size_t total_sent = 0;

  std::string serialized = res.get_serialized();

  while (total_sent < serialized.size()) {
    ssize_t sent = send(client_socket_, serialized.data() + total_sent,
                        serialized.size() - total_sent, 0);

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

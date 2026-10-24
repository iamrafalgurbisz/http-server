#include "server/server.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <iostream>
#include <string>
#include <system_error>

#include "http/http-method.hpp"
#include "http/http-request.hpp"
#include "http/http-response.hpp"
#include "router/router.hpp"

Server::Server(const Router& router) : router_(router) {};

volatile std::sig_atomic_t Server::running_ = 0;

void Server::handle_signal_(int sig) { running_ = 0; }

void Server::install_signal_handlers_()
{
  struct sigaction sa{};
  sa.sa_handler = handle_signal_;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigaction(SIGINT, &sa, nullptr);
  sigaction(SIGTERM, &sa, nullptr);

  std::signal(SIGPIPE, SIG_IGN);
};

void Server::start(int PORT)
{
  running_ = 1;
  install_signal_handlers_();

  constexpr std::size_t MAX_BODY_SIZE = 32 * 1024;

  server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket_ < 0)
  {
    throw std::system_error(errno, std::generic_category(), "server_socket");
  }

  int opt = 1;
  int sockopt =
      setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (sockopt < 0)
  {
    throw std::system_error(errno, std::generic_category(), "setsockopt");
  }

  sockaddr_in server_address;
  server_address.sin_family      = AF_INET;
  server_address.sin_port        = htons(PORT);
  server_address.sin_addr.s_addr = INADDR_ANY;

  int bind_res = bind(server_socket_, (struct sockaddr*)&server_address,
                      sizeof(server_address));

  if (bind_res < 0)
  {
    throw std::system_error(errno, std::generic_category(), "bind");
  }

  int listener = listen(server_socket_, 5);

  if (listener < 0)
  {
    throw std::system_error(errno, std::generic_category(), "listen");
  }

  std::cout << "Listening on port " << PORT << "..." << "\n";

  loop_();
}

void Server::loop_()
{
  while (running_)
  {
    bool already_responded = false;
    client_socket_         = accept(server_socket_, nullptr, nullptr);

    if (client_socket_ < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }

      if (errno == ECONNABORTED)
      {
        continue;
      }

      if (errno == EMFILE)
      {
        continue;
      }

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
    while (true)
    {
      char temp[4096];

      ssize_t n = recv(client_socket_, temp, sizeof(temp), 0);

      if (n == 0)
      {
        break;
      }

      if (n < 0)
      {
        if (errno == EINTR)
        {
          if (!running_)
          {
            break;
          }
          continue;
        }

        std::perror("recv");
        break;
      }

      buffer.append(temp, n);

      HttpRequestParseResult parse_result = req.parse(buffer);

      std::cout << "[" << req.method().str() << "] " << req.path() << "\n";

      if (parse_result == HttpRequestParseResult::Incomplete)
      {
        continue;
      }

      if (parse_result == HttpRequestParseResult::UriTooLarge)
      {
        respond_(HttpResponse::UriTooLarge());

        break;
      }

      if (parse_result == HttpRequestParseResult::HeadersTooLarge)
      {
        respond_(HttpResponse::HeadersTooLarge());

        break;
      }

      if (parse_result == HttpRequestParseResult::BodyTooLarge)
      {
        respond_(HttpResponse::BodyTooLarge());

        break;
      }

      if (parse_result == HttpRequestParseResult::UnknownExpectation)
      {
        respond_(HttpResponse::ExpectationFailed());

        break;
      }

      if (parse_result == HttpRequestParseResult::Invalid)
      {
        respond_(HttpResponse::BadRequest());

        break;
      }

      if (!req.version().ends_with("1.1"))
      {
        respond_(HttpResponse::HttpVersionNotSupported());
      }

      if (req.expect() == Expectation::Continue)
      {
        auto res = HttpResponse::Continue();

        if (!already_responded)
        {
          respond_(res);
          already_responded = true;
        }

        if (parse_result == HttpRequestParseResult::Incomplete)
        {
          continue;
        }
      }

      if (req.method().key() == HttpMethodKey::Options)
      {
        std::vector<std::string> options = router_.get_options_(req.path());

        auto res = HttpResponse::OkNoContent();

        std::string opts;

        for (const auto& option : options)
        {
          if (!opts.empty())
          {
            opts += ", ";
          }

          opts += option;
        }

        res.header(HeaderKey::Allow, opts);

        respond_(res);

        break;
      }

      if (router_.handlers().find(req.path()) != router_.handlers().end())
      {
        const auto& path_handlers = router_.handlers().find(req.path())->second;

        if (req.method().key() == HttpMethodKey::Head &&
            path_handlers.find(HttpMethodKey::Get) != path_handlers.end())
        {
          std::cout << "HEAD request " << req.path() << " was mapped to GET\n";

          HttpResponse res =
              path_handlers.find(HttpMethodKey::Get)->second(req);

          respond_(res);

          break;
        }

        if (path_handlers.find(req.method().key()) == path_handlers.end())
        {
          auto res = HttpResponse::MethodNotAllowed();

          respond_(res);

          break;
        }

        HttpResponse res = path_handlers.find(req.method().key())->second(req);

        respond_(res);

        break;
      }
      else
      {
        auto res = HttpResponse::NotFound();

        respond_(res);

        break;
      }
    }

    ::close(client_socket_);
  }
}

void Server::respond_(const HttpResponse& res)
{
  size_t total_sent = 0;

  std::string serialized = res.get_serialized();

  while (total_sent < serialized.size())
  {
    ssize_t sent = send(client_socket_, serialized.data() + total_sent,
                        serialized.size() - total_sent, 0);

    if (sent < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }

      if (errno == EPIPE || errno == ECONNRESET)
      {
        return;
      }

      throw std::system_error(errno, std::generic_category(), "send");
    }

    if (sent == 0)
    {
      throw std::runtime_error("send returned 0");
    }

    total_sent += static_cast<size_t>(sent);
  }
}

void Server::close()
{
  if (client_socket_ >= 0)
  {
    ::close(client_socket_);
    client_socket_ = -1;
  }

  if (server_socket_ >= 0)
  {
    ::close(server_socket_);
    server_socket_ = -1;
  }
}

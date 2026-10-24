#pragma once

#include <csignal>

#include "http/http-response.hpp"
#include "router/router.hpp"

class Server
{
 public:
  Server(const Router& router);

  void start(int PORT);
  void close();

 private:
  int                               server_socket_ = -1;
  int                               client_socket_ = -1;
  static volatile std::sig_atomic_t running_;
  Router                            router_;

  void        loop_();
  void        respond_(const HttpResponse& res);
  static void install_signal_handlers_();
  static void handle_signal_(int);
};

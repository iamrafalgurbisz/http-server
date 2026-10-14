#pragma once

#include "http/http-method.hpp"
#include "http/http-request.hpp"
#include "http/http-response.hpp"
#include <functional>
#include <map>
#include <string>

using Handler = std::function<HttpResponse(const HttpRequest &)>;

class Router {
public:
  void get(std::string, Handler);
  void head(std::string, Handler);
  void post(std::string, Handler);
  const std::map<std::string, Handler> &handlers() const;

private:
  Handler wrap_handler_(HttpMethod method, Handler handler);
  std::map<std::string, std::function<HttpResponse(const HttpRequest &req)>>
      handlers_;
};

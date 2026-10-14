#pragma once

#include "http/http-method.hpp"
#include "http/http-request.hpp"
#include "http/http-response.hpp"
#include <functional>
#include <map>
#include <string>

using Handler = std::function<HttpResponse(const HttpRequest &)>;
using Handlers = std::map<std::string, std::map<HttpMethodKey, Handler>>;

class Router {
public:
  void get(std::string, Handler);
  void head(std::string, Handler);
  void post(std::string, Handler);
  const Handlers &handlers() const;

private:
  void apply_handler_(HttpMethod method, std::string path, Handler handler);
  Handler wrap_handler_(HttpMethod method, Handler handler);
  Handlers handlers_;
};

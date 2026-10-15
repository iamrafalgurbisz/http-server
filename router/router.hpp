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
  void get(const std::string &, const Handler &);
  void head(const std::string &, const Handler &);
  void post(const std::string &, const Handler &);
  const Handlers &handlers() const;

private:
  void apply_handler_(const HttpMethod &method, const std::string &path,
                      const Handler &handler);
  Handler wrap_handler_(const HttpMethod &method, const Handler &handler);
  Handlers handlers_;
};

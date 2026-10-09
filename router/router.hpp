#pragma once

#include "http/http-response.hpp"
#include <functional>
#include <map>
#include <string>

class Router {
public:
  void get(std::string path, std::function<HttpResponse()> handler);
  const std::map<std::string, std::function<HttpResponse()>> &handlers() const;

private:
  std::map<std::string, std::function<HttpResponse()>> handlers_;
};

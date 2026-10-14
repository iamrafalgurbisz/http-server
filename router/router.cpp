#include "router.hpp"
#include "http/http-method.hpp"
#include "http/http-request.hpp"
#include "http/http-response.hpp"
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

void Router::get(std::string path, Handler handler) {
  HttpMethod method("GET");

  std::string handler_key = method.str() + path;

  if (handlers_.find(handler_key) != handlers_.end()) {
    throw std::runtime_error("Error: Path " + path + " was reused.");
  }

  handlers_.insert({handler_key, wrap_handler_(method, handler)});
}

void Router::post(std::string path, Handler handler) {
  HttpMethod method("POST");

  std::string handler_key = method.str() + path;

  if (handlers_.find(handler_key) != handlers_.end()) {
    throw std::runtime_error("Error: Path " + path + " was reused.");
  }

  handlers_.insert({handler_key, wrap_handler_(method, handler)});
}

void Router::head(std::string path, Handler handler) {
  HttpMethod method("HEAD");

  std::string handler_key = method.str() + path;

  if (handlers_.find(handler_key) != handlers_.end()) {
    throw std::runtime_error("Error: Path " + path + " was reused.");
  }

  handlers_.insert({handler_key, wrap_handler_(method, handler)});
}

Handler Router::wrap_handler_(HttpMethod method, Handler handler) {
  return [method, handler](const HttpRequest &req) -> HttpResponse {
    HttpResponse res = handler(req);

    if (method.key() == HttpMethodKey::Head && res.has_body()) {
      std::cout << "Warning: HEAD response should NOT have body\n";

      res.clear_body();
    }

    if (req.method().key() == HttpMethodKey::Head && res.has_body()) {
      std::cout << "Info: HEAD response body was cleared\n";

      res.clear_body();
    }

    return res;
  };
};

const std::map<std::string, std::function<HttpResponse(const HttpRequest &)>> &
Router::handlers() const {
  return handlers_;
}

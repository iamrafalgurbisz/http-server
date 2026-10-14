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

  apply_handler_(method, path, handler);
}

void Router::post(std::string path, Handler handler) {
  HttpMethod method("POST");

  apply_handler_(method, path, handler);
}

void Router::head(std::string path, Handler handler) {
  HttpMethod method("HEAD");

  apply_handler_(method, path, handler);
}

void Router::apply_handler_(HttpMethod method, std::string path,
                            Handler handler) {
  bool path_exists = handlers_.find(path) != handlers_.end();

  if (!path_exists) {
    std::map<HttpMethodKey, Handler> new_handlers;
    new_handlers.insert({method.key(), wrap_handler_(method, handler)});

    handlers_.insert({path, new_handlers});

    return;
  }

  auto &found_handlers = handlers_.find(path)->second;
  bool method_exists =
      found_handlers.find(method.key()) != found_handlers.end();

  if (path_exists && method_exists) {
    throw std::runtime_error("Error: Path " + path + " was reused.");
  }

  if (path_exists && !method_exists) {
    found_handlers.insert({method.key(), wrap_handler_(method, handler)});

    return;
  }

  throw std::runtime_error("Error: apply_handler");
}

Handler Router::wrap_handler_(HttpMethod method, Handler handler) {
  return [method, handler](const HttpRequest &req) -> HttpResponse {
    HttpResponse res = handler(req);

    std::cout << "wrapper, " << method.str() << ", " << req.method().str()
              << "\n";

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

const Handlers &Router::handlers() const { return handlers_; }

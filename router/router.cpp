#include "router.hpp"
#include "http/http-response.hpp"
#include <map>
#include <stdexcept>
#include <string>

void Router::get(std::string path, std::function<HttpResponse()> handler) {
  if (handlers_.find(path) != handlers_.end()) {
    throw std::runtime_error("Error: Path " + path + " was reused.");
  }

  handlers_.insert({path, handler});
}

const std::map<std::string, std::function<HttpResponse()>> &
Router::handlers() const {
  return handlers_;
}

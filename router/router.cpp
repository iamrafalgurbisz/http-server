#include "router.hpp"

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "http/http-method.hpp"
#include "http/http-request.hpp"
#include "http/http-response.hpp"
#include "logger/logger.hpp"

void Router::get(const std::string& path, const Handler& handler)
{
  HttpMethod method("GET");

  apply_handler_(method, path, handler);
}

void Router::post(const std::string& path, const Handler& handler)
{
  HttpMethod method("POST");

  apply_handler_(method, path, handler);
}

void Router::head(const std::string& path, const Handler& handler)
{
  HttpMethod method("HEAD");

  apply_handler_(method, path, handler);
}

void Router::apply_handler_(const HttpMethod& method, const std::string& path,
                            const Handler& handler)
{
  bool path_exists = handlers_.find(path) != handlers_.end();

  if (!path_exists)
  {
    std::map<HttpMethodKey, Handler> new_handlers;
    new_handlers.insert({method.key(), wrap_handler_(method, handler)});

    handlers_.insert({path, new_handlers});

    return;
  }

  auto& found_handlers = handlers_.find(path)->second;
  bool  method_exists =
      found_handlers.find(method.key()) != found_handlers.end();

  if (path_exists && method_exists)
  {
    throw std::runtime_error("Error: Path " + path + " was reused.");
  }

  if (path_exists && !method_exists)
  {
    found_handlers.insert({method.key(), wrap_handler_(method, handler)});

    return;
  }

  throw std::runtime_error("Error: apply_handler");
}

Handler Router::wrap_handler_(const HttpMethod& method, const Handler& handler)
{
  return [method, handler](const HttpRequest& req) -> HttpResponse
  {
    HttpResponse res = handler(req);

    if (method.key() == HttpMethodKey::Head && res.has_body())
    {
      Logger::warning() << "HEAD response should NOT have body";

      res.clear_body();
    }

    if (req.method().key() == HttpMethodKey::Head && res.has_body())
    {
      Logger::info() << "HEAD response body was cleared";

      res.clear_body();
    }

    return res;
  };
};

std::vector<std::string> Router::get_options_(const std::string& path)
{
  const auto& path_it = handlers_.find(path);

  if (path_it == handlers_.end())
  {
    return std::vector<std::string>(0) = {};
  }

  auto                                       path_handlers = path_it->second;
  std::map<HttpMethodKey, Handler>::iterator path_handlers_it;

  std::vector<std::string> options;

  for (path_handlers_it = path_handlers.begin();
       path_handlers_it != path_handlers.end(); path_handlers_it++)
  {
    options.push_back(HttpMethod(path_handlers_it->first).str());
  }

  return options;
};

const Handlers& Router::handlers() const { return handlers_; }

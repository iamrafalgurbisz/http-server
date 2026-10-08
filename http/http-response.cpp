#include "http-response.hpp"
#include <string>

HttpResponse::HttpResponse(HttpStatus status) {
  status_ = status;

  serialize();
}

void HttpResponse::serialize() {
  std::string res;

  res += "HTTP/1.1 ";
  res += std::to_string(static_cast<int>(status_));
  res += "\r\n";
  res += "Content-Type: text/plain; charset=utf-8\r\n";
  res += "Content-Length: 0\r\n";
  res += "Connection: close\r\n";
  res += "\r\n";

  response_ = res;
}

std::string HttpResponse::get_status_line() const {
  switch (status_) {
  case HttpStatus::OK:
    return std::to_string(static_cast<int>(status_)) + " OK";
  case HttpStatus::NOT_FOUND:
    return std::to_string(static_cast<int>(status_)) + " Not Found";
  }

  return "500 Internal Server Error";
};

const std::string &HttpResponse::response() const { return response_; }

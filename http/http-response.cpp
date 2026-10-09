#include "http-response.hpp"
#include <string>

HttpResponse::HttpResponse(HttpStatus status, std::string_view body) {
  status_ = status;
  body_ = body;

  serialize();
}

void HttpResponse::serialize() {
  std::string res;

  res += "HTTP/1.1 ";
  res += std::to_string(static_cast<int>(status_));
  res += "\r\n";
  res += "Content-Type: text/plain; charset=utf-8\r\n";

  if (!body_.empty()) {
    res += "Content-Length: " + std::to_string(body_.size()) + "\r\n";
  } else {
    res += "Content-Length: 0\r\n";
  }

  res += "Connection: close\r\n";
  res += "\r\n";

  if (!body_.empty()) {
    res += body_;
  }

  response_ = res;
}

std::string HttpResponse::get_status_line() const {
  switch (status_) {
  case HttpStatus::OK:
    return std::to_string(static_cast<int>(status_)) + " OK";
  case HttpStatus::NOT_FOUND:
    return std::to_string(static_cast<int>(status_)) + " Not Found";
  case HttpStatus::BAD_REQUEST:
    return std::to_string(static_cast<int>(status_)) + " Bad Request";
  }

  return "500 Internal Server Error";
};

const std::string &HttpResponse::response() const { return response_; }

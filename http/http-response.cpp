#include "http-response.hpp"
#include <iostream>
#include <string>

HttpResponse::HttpResponse(HttpStatus status, std::string_view body) {
  status_ = status;

  if (status == HttpStatus::CONTINUE && !body.empty()) {
    std::cout << "Warning: HTTP 1xx responses cannot have body\n";

    clear_body();

    return;
  }

  body_ = body;
}

void HttpResponse::clear_body() {
  if (skip_body_) {
    std::cout << "Warning: body already skipped\n";
  }

  skip_body_ = true;
}

const std::string HttpResponse::get_serialized() const {
  std::string res;

  res += "HTTP/1.1 ";
  res += get_status_line_();
  res += "\r\n";
  res += "Content-Type: text/plain; charset=utf-8\r\n";

  if (!body_.empty()) {
    res += "Content-Length: " + std::to_string(body_.size()) + "\r\n";
  } else {
    res += "Content-Length: 0\r\n";
  }

  res += "Connection: close\r\n";
  res += "\r\n";

  if (!body_.empty() && !skip_body_) {
    res += body_;
  }

  return res;
}

std::string HttpResponse::get_status_line_() const {
  switch (status_) {
  case HttpStatus::CONTINUE:
    return std::to_string(static_cast<int>(status_)) + " Continue";
  case HttpStatus::OK:
    return std::to_string(static_cast<int>(status_)) + " OK";
  case HttpStatus::BAD_REQUEST:
    return std::to_string(static_cast<int>(status_)) + " Bad Request";
  case HttpStatus::NOT_FOUND:
    return std::to_string(static_cast<int>(status_)) + " Not Found";
  case HttpStatus::METHOD_NOT_ALLOWED:
    return std::to_string(static_cast<int>(status_)) + " Method Not Allowed";
  case HttpStatus::BODY_TOO_LARGE:
    return std::to_string(static_cast<int>(status_)) + " Content Too Large";
  case HttpStatus::URI_TOO_LARGE:
    return std::to_string(static_cast<int>(status_)) + " URI Too Long";
  case HttpStatus::EXPECTATION_FAILED:
    return std::to_string(static_cast<int>(status_)) + " Expectation Failed";
  case HttpStatus::HEADERS_TOO_LARGE:
    return std::to_string(static_cast<int>(status_)) +
           " Request Header Fields Too Large";
  case HttpStatus::INTERNAL_SERVER_ERROR:
    return std::to_string(static_cast<int>(status_)) + " Internal Server Error";
  case HttpStatus::HTTP_VERSION_NOT_SUPPORTED:
    return std::to_string(static_cast<int>(status_)) +
           " HTTP Version Not Supported";
  }

  return std::to_string(static_cast<int>(HttpStatus::INTERNAL_SERVER_ERROR)) +
         "Internal Server Error";
};

bool HttpResponse::has_body() const { return !body_.empty(); }

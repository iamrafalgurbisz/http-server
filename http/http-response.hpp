#pragma once

#include <string>

enum class HttpStatus { OK = 200, NOT_FOUND = 404, BAD_REQUEST = 400 };

class HttpResponse {
public:
  explicit HttpResponse(HttpStatus status, std::string_view body = {});
  const std::string &response() const;

private:
  void serialize();
  HttpStatus status_;
  std::string response_;
  std::string body_;
  std::string get_status_line() const;
};

#pragma once

#include <string>

enum class HttpStatus { OK = 200, NOT_FOUND = 404 };

class HttpResponse {
public:
  HttpResponse(HttpStatus status);
  const std::string &response() const;

private:
  void serialize();
  HttpStatus status_;
  std::string response_;
  std::string get_status_line() const;
};

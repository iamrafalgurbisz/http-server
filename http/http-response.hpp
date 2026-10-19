#pragma once

#include <string>

enum class HttpStatus {
  CONTINUE = 100,
  OK = 200,
  BAD_REQUEST = 400,
  NOT_FOUND = 404,
  METHOD_NOT_ALLOWED = 405,
  BODY_TOO_LARGE = 413,
  URI_TOO_LARGE = 414,
  EXPECTATION_FAILED = 417,
  HEADERS_TOO_LARGE = 431,
  INTERNAL_SERVER_ERROR = 500,
  HTTP_VERSION_NOT_SUPPORTED = 505
};

class HttpResponse {
public:
  explicit HttpResponse(HttpStatus status, std::string_view body = {});
  bool has_body() const;
  const std::string get_serialized() const;
  void clear_body();

private:
  HttpStatus status_;
  std::string body_;
  bool skip_body_ = false;
  std::string get_status_line_() const;
};

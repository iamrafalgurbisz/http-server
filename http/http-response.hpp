#pragma once

#include <string>

#include "http/http-request.hpp"

enum class HttpStatus
{
  CONTINUE                   = 100,
  OK                         = 200,
  OK_NO_CONTENT              = 204,
  BAD_REQUEST                = 400,
  NOT_FOUND                  = 404,
  METHOD_NOT_ALLOWED         = 405,
  BODY_TOO_LARGE             = 413,
  URI_TOO_LARGE              = 414,
  EXPECTATION_FAILED         = 417,
  HEADERS_TOO_LARGE          = 431,
  INTERNAL_SERVER_ERROR      = 500,
  HTTP_VERSION_NOT_SUPPORTED = 505
};

class HttpResponse
{
 public:
  static HttpResponse Continue();
  static HttpResponse Ok();
  static HttpResponse OkNoContent();
  static HttpResponse BadRequest();
  static HttpResponse NotFound();
  static HttpResponse MethodNotAllowed();
  static HttpResponse BodyTooLarge();
  static HttpResponse UriTooLarge();
  static HttpResponse ExpectationFailed();
  static HttpResponse HeadersTooLarge();
  static HttpResponse InternalServerError();
  static HttpResponse HttpVersionNotSupported();

  HttpResponse&  header(HeaderKey, std::string_view) &;
  HttpResponse&& header(HeaderKey, std::string_view) &&;

  HttpResponse&  body(std::string_view) &;
  HttpResponse&& body(std::string_view) &&;

  bool              has_body() const;
  const std::string get_serialized() const;
  void              clear_body();

 private:
  HttpResponse(HttpStatus status);

  HttpStatus  status_;
  std::string body_;
  bool        skip_body_ = false;
  HttpHeaders headers_;
  std::string get_status_line_() const;
};

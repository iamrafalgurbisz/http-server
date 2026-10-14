#pragma once

#include "http/http-method.hpp"
#include <string>

enum class Expectation { Continue, Unknown };

enum class HttpRequestParseResult {
  Incomplete,
  Invalid,
  Complete,
  UriTooLarge,
  HeadersTooLarge,
  BodyTooLarge,
  UnknownExpectation
};

enum class HeaderKey {
  Host,
  UserAgent,
  Accept,
  ContentLength,
  ContentType,
  Expect,
  Unknown
};

class HttpRequest {
public:
  const HttpRequestParseResult parse(std::string_view buffer);
  const HttpMethod &method() const;
  const std::string &path() const;
  const std::string &version() const;
  const std::string &host() const;
  const std::string &user_agent() const;
  const std::string &accept() const;
  const Expectation &expect() const;
  const size_t &content_length() const;
  const std::string &content_type() const;
  const std::string &body() const;

private:
  static HeaderKey parse_header_key_(std::string_view key);
  static Expectation parse_expectation_(std::string_view str);
  HttpMethod method_;
  std::string path_;
  std::string version_;
  std::string host_;
  std::string user_agent_;
  std::string accept_;
  Expectation expect_;
  size_t content_length_ = 0;
  std::string content_type_;
  std::string body_;
};

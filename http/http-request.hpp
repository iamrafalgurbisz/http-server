#include <string>

enum class HttpRequestParseResult { Incomplete, Invalid, Complete };

enum class HeaderKey {
  Host,
  UserAgent,
  Accept,
  ContentLength,
  ContentType,
  Unknown
};

class HttpRequest {
public:
  const HttpRequestParseResult parse(std::string_view buffer);
  const std::string &method() const;
  const std::string &version() const;
  const std::string &host() const;
  const std::string &user_agent() const;
  const std::string &accept() const;
  const size_t &content_length() const;
  const std::string &content_type() const;
  const std::string &body() const;

private:
  static HeaderKey parse_header_key(std::string_view key);
  std::string method_;
  std::string version_;
  std::string host_;
  std::string user_agent_;
  std::string accept_;
  size_t content_length_;
  std::string content_type_;
  std::string body_;
};

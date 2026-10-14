#include "http-request.hpp"
#include "http/http-method.hpp"
#include <charconv>
#include <iostream>
#include <system_error>

const HttpRequestParseResult HttpRequest::parse(std::string_view buffer) {
  constexpr std::size_t MAX_REQUEST_LINE_SIZE = 8 * 1024;
  constexpr std::size_t MAX_HEADERS_SIZE = 16 * 1024;
  constexpr std::size_t MAX_BODY_SIZE = 32 * 1024;

  size_t req_pos = buffer.find("\r\n");

  if (req_pos == std::string_view::npos) {
    return HttpRequestParseResult::Incomplete;
  }

  /* INFO: REQUEST LINE */
  std::string_view req(buffer.data(), req_pos);

  size_t req_first_space = req.find(' ');
  if (req_first_space == std::string_view::npos) {
    std::cout << "Invalid first space\n";

    return HttpRequestParseResult::Invalid;
  }

  size_t req_second_space = req.find(' ', req_first_space + 1);
  if (req_second_space == std::string_view::npos) {
    std::cout << "Invalid string\n";

    return HttpRequestParseResult::Invalid;
  }

  if (req.find(' ', req_second_space + 1) != std::string_view::npos) {
    std::cout << "Invalid spacing\n";

    return HttpRequestParseResult::Invalid;
  }

  HttpMethod method(req.substr(0, req_first_space));

  if (!method.is_valid()) {
    std::cout << "Invalid method\n";

    return HttpRequestParseResult::Invalid;
  }

  if (req.substr(req_first_space + 1, req_second_space - req_first_space - 1)
          .size() > MAX_REQUEST_LINE_SIZE) {
    return HttpRequestParseResult::UriTooLarge;
  }

  method_ = method;
  path_ =
      req.substr(req_first_space + 1, req_second_space - req_first_space - 1);
  version_ = req.substr(req_second_space + 1);

  if (path_.empty() || version_.empty()) {
    std::cout << "Invalid path and/or version\n";

    return HttpRequestParseResult::Invalid;
  }

  /* INFO: HEADERS */
  size_t headers_end_pos = buffer.find("\r\n\r\n", req_pos + 2);

  if (headers_end_pos == std::string_view::npos) {
    return HttpRequestParseResult::Incomplete;
  }

  size_t i = req_pos + 2;

  if (headers_end_pos - i > MAX_HEADERS_SIZE) {
    return HttpRequestParseResult::HeadersTooLarge;
  }

  while (i < headers_end_pos) {
    size_t pos = buffer.find("\r\n", i);

    if (pos == std::string::npos) {
      break;
    }

    std::string_view header(buffer.data() + i, pos - i);

    size_t separator = header.find(":");

    if (separator == std::string_view::npos) {
      std::cout << "Invalid separator\n";

      return HttpRequestParseResult::Invalid;
    }

    std::string_view header_raw_key = header.substr(0, separator);
    std::string_view header_value = header.substr(separator + 2);

    HeaderKey header_key = parse_header_key_(header_raw_key);

    switch (header_key) {
    case HeaderKey::Host:
      host_ = header_value;
      break;

    case HeaderKey::UserAgent:
      user_agent_ = header_value;
      break;

    case HeaderKey::Accept:
      accept_ = header_value;
      break;

    case HeaderKey::ContentLength: {
      auto result = std::from_chars(header_value.data(),
                                    header_value.data() + header_value.size(),
                                    content_length_);

      if (result.ec != std::errc{} ||
          result.ptr != header_value.data() + header_value.size()) {
        std::cout << "Invalid something \n";

        return HttpRequestParseResult::Invalid;
      }

      break;
    }

    case HeaderKey::ContentType:
      content_type_ = header_value;
      break;

    case HeaderKey::Expect: {
      expect_ = parse_expectation_(header_value);

      break;
    }

    case HeaderKey::Unknown:
      break;
    }

    i = pos + 2;
  }

  if (host_.empty()) {
    std::cout << "Invalid host\n";

    return HttpRequestParseResult::Invalid;
  }

  if (expect_ == Expectation::Unknown) {
    return HttpRequestParseResult::UnknownExpectation;
  }

  size_t body_start = headers_end_pos + 4;
  size_t received_body = buffer.size() - body_start;

  if (received_body < content_length_) {
    return HttpRequestParseResult::Incomplete;
  }

  if (buffer.substr(body_start, content_length_).size() > MAX_BODY_SIZE) {
    return HttpRequestParseResult::BodyTooLarge;
  }

  body_ = buffer.substr(body_start, content_length_);

  return HttpRequestParseResult::Complete;
}

HeaderKey HttpRequest::parse_header_key_(std::string_view key) {
  if (key == "Host") {
    return HeaderKey::Host;
  }

  if (key == "User-Agent") {
    return HeaderKey::UserAgent;
  }

  if (key == "Accept") {
    return HeaderKey::Accept;
  }

  if (key == "Expect") {
    return HeaderKey::Expect;
  }

  if (key == "Content-Length") {
    return HeaderKey::ContentLength;
  }

  if (key == "Content-Type") {
    return HeaderKey::ContentType;
  }

  return HeaderKey::Unknown;
}

Expectation HttpRequest::parse_expectation_(std::string_view str) {
  if (str == "100-continue") {
    return Expectation::Continue;
  }

  return Expectation::Unknown;
}

const HttpMethod &HttpRequest::method() const { return method_; }
const std::string &HttpRequest::path() const { return path_; }
const std::string &HttpRequest::version() const { return version_; }
const std::string &HttpRequest::host() const { return host_; }
const std::string &HttpRequest::user_agent() const { return user_agent_; }
const std::string &HttpRequest::accept() const { return accept_; }
const Expectation &HttpRequest::expect() const { return expect_; }
const size_t &HttpRequest::content_length() const { return content_length_; }
const std::string &HttpRequest::content_type() const { return content_type_; }
const std::string &HttpRequest::body() const { return body_; }

#include "http-request.hpp"
#include <charconv>
#include <system_error>

const HttpRequestParseResult HttpRequest::parse(std::string_view buffer) {
  size_t req_pos = buffer.find("\r\n");

  if (req_pos == std::string_view::npos) {
    return HttpRequestParseResult::Incomplete;
  }

  /* INFO: REQUEST LINE */
  std::string_view req(buffer.data(), req_pos);

  size_t req_separator_pos = req.find("/");

  if (req_separator_pos == std::string_view::npos) {
    return HttpRequestParseResult::Invalid;
  }

  method_ = req.substr(0, req_separator_pos);
  version_ = req.substr(req_separator_pos + 2);

  /* INFO: HEADERS */
  size_t headers_end_pos = buffer.find("\r\n\r\n", req_pos + 2);

  if (headers_end_pos == std::string_view::npos) {
    return HttpRequestParseResult::Incomplete;
  }

  size_t i = req_pos + 2;

  while (i < headers_end_pos) {
    size_t pos = buffer.find("\r\n", i);

    if (pos == std::string::npos) {
      break;
    }

    std::string_view header(buffer.data() + i, pos - i);

    size_t separator = header.find(":");

    if (separator == std::string_view::npos) {
      return HttpRequestParseResult::Invalid;
    }

    std::string_view header_raw_key = header.substr(0, separator);
    std::string_view header_value = header.substr(separator + 2);

    HeaderKey header_key = parse_header_key(header_raw_key);

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

        return HttpRequestParseResult::Invalid;
      }

      break;
    }

    case HeaderKey::ContentType:
      content_type_ = header_value;
      break;

    case HeaderKey::Unknown:
      break;
    }

    i = pos + 2;
  }

  size_t body_start = headers_end_pos + 4;
  size_t received_body = buffer.size() - body_start;

  if (received_body < content_length_) {
    return HttpRequestParseResult::Incomplete;
  }

  body_ = buffer.substr(body_start, content_length_);

  return HttpRequestParseResult::Complete;

  // std::cout << buffer << "\n";
}

HeaderKey HttpRequest::parse_header_key(std::string_view key) {
  if (key == "Host") {
    return HeaderKey::Host;
  }

  if (key == "User-Agent") {
    return HeaderKey::UserAgent;
  }

  if (key == "Accept") {
    return HeaderKey::Accept;
  }

  if (key == "Content-Length") {
    return HeaderKey::ContentLength;
  }

  if (key == "Content-Type") {
    return HeaderKey::ContentType;
  }

  return HeaderKey::Unknown;
}

const std::string &HttpRequest::method() const { return method_; }
const std::string &HttpRequest::version() const { return version_; }
const std::string &HttpRequest::host() const { return host_; }
const std::string &HttpRequest::user_agent() const { return user_agent_; }
const std::string &HttpRequest::accept() const { return accept_; }
const size_t &HttpRequest::content_length() const { return content_length_; }
const std::string &HttpRequest::content_type() const { return content_type_; }
const std::string &HttpRequest::body() const { return body_; }

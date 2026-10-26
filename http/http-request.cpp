#include "http/http-request.hpp"

#include <algorithm>
#include <charconv>
#include <string>
#include <system_error>

#include "logger/logger.hpp"
#include "utils/string/trim.hpp"

const HttpRequestParseResult HttpRequest::parse(std::string_view buffer)
{
  constexpr std::size_t MAX_REQUEST_LINE_SIZE = 8 * 1024;
  constexpr std::size_t MAX_HEADERS_SIZE      = 16 * 1024;
  constexpr std::size_t MAX_BODY_SIZE         = 32 * 1024;

  size_t req_pos = buffer.find("\r\n");

  if (req_pos == std::string_view::npos)
  {
    if (buffer.size() > MAX_REQUEST_LINE_SIZE)
    {
      return HttpRequestParseResult::UriTooLarge;
    }

    return HttpRequestParseResult::Incomplete;
  }

  /* INFO: REQUEST LINE */
  std::string_view req(buffer.data(), req_pos);

  size_t req_first_space = req.find(' ');
  if (req_first_space == std::string_view::npos)
  {
    Logger::error() << "Invalid first space";

    return HttpRequestParseResult::Invalid;
  }

  size_t req_second_space = req.find(' ', req_first_space + 1);
  if (req_second_space == std::string_view::npos)
  {
    Logger::error() << "Invalid string";

    return HttpRequestParseResult::Invalid;
  }

  if (req.find(' ', req_second_space + 1) != std::string_view::npos)
  {
    Logger::error() << "Invalid spacing";

    return HttpRequestParseResult::Invalid;
  }

  HttpMethod method(req.substr(0, req_first_space));

  if (!method.is_valid())
  {
    Logger::error() << "Invalid method";

    return HttpRequestParseResult::Invalid;
  }

  if (req.substr(req_first_space + 1, req_second_space - req_first_space - 1)
          .size() > MAX_REQUEST_LINE_SIZE)
  {
    return HttpRequestParseResult::UriTooLarge;
  }

  method_ = method;
  path_ =
      req.substr(req_first_space + 1, req_second_space - req_first_space - 1);
  version_ = req.substr(req_second_space + 1);

  if (path_.empty() || version_.empty())
  {
    Logger::error() << "Invalid path and/or version";

    return HttpRequestParseResult::Invalid;
  }

  if (version_ != "HTTP/1.1")
  {
    return HttpRequestParseResult::InvalidHttpVersion;
  }

  /* INFO: HEADERS */
  const size_t headers_start   = req_pos + 2;
  const size_t headers_end_pos = buffer.find("\r\n\r\n", req_pos);

  if (headers_end_pos == std::string_view::npos)
  {
    if (buffer.size() - headers_start > MAX_HEADERS_SIZE)
    {
      return HttpRequestParseResult::HeadersTooLarge;
    }

    return HttpRequestParseResult::Incomplete;
  }

  const size_t headers_size =
      headers_end_pos > req_pos ? headers_end_pos - headers_start : 0;

  if (headers_size > MAX_HEADERS_SIZE)
    return HttpRequestParseResult::HeadersTooLarge;

  std::string_view headers_block = buffer.substr(headers_start, headers_size);
  size_t           i             = headers_start;

  while (i < headers_end_pos)
  {
    size_t pos = buffer.find("\r\n", i);

    if (pos == std::string::npos)
    {
      break;
    }

    std::string_view header(buffer.data() + i, pos - i);

    size_t separator = header.find(":");

    if (separator == std::string_view::npos)
    {
      Logger::error() << "Invalid separator";

      return HttpRequestParseResult::Invalid;
    }

    std::string_view header_raw_key = header.substr(0, separator);
    std::string_view header_value   = trim(header.substr(separator + 1));

    HeaderKey header_key = parse_header_key_(header_raw_key);

    switch (header_key)
    {
      case HeaderKey::Host:
        if (!host_.empty())
        {
          Logger::error() << "Duplicate Host header";

          return HttpRequestParseResult::Invalid;
        }

        if (header_value.empty())
        {
          Logger::error() << "Invalid Host header value";

          return HttpRequestParseResult::Invalid;
        }

        host_ = header_value;

        break;

      case HeaderKey::UserAgent:
        user_agent_ = header_value;

        break;

      case HeaderKey::Accept:
        accept_ = header_value;

        break;

      case HeaderKey::Allow:
        allow_ = header_value;

        break;

      case HeaderKey::ContentLength:
      {
        // TODO: handle header duplicate

        std::size_t result;

        auto parse_result =
            std::from_chars(header_value.data(),
                            header_value.data() + header_value.size(), result);

        if (parse_result.ec != std::errc{} ||
            parse_result.ptr != header_value.data() + header_value.size())
        {
          Logger::error() << "Invalid Content-Length header value";

          return HttpRequestParseResult::Invalid;
        }

        if (result > MAX_BODY_SIZE)
        {
          return HttpRequestParseResult::BodyTooLarge;
        }

        content_length_ = result;

        break;
      }

      case HeaderKey::ContentType:
        if (!content_type_.empty())
        {
          return HttpRequestParseResult::Invalid;
        }

        content_type_ = header_value;

        break;

      case HeaderKey::Expect:
      {
        Expectation parsed_expect = parse_expectation_(header_value);

        if (parsed_expect == Expectation::Unknown)
        {
          return HttpRequestParseResult::UnknownExpectation;
        }

        expect_ = parsed_expect;

        break;
      }

      case HeaderKey::Unknown:
        break;
    }

    i = pos + 2;
  }

  if (expect_ == Expectation::Continue)
  {
    return HttpRequestParseResult::WaitingForContinue;
  }

  if (host_.empty())
  {
    return HttpRequestParseResult::Invalid;
  }

  size_t body_start    = headers_end_pos + 4;
  size_t received_body = buffer.size() - body_start;

  if (received_body < content_length_)
  {
    if (received_body > MAX_BODY_SIZE)
    {
      return HttpRequestParseResult::BodyTooLarge;
    }

    return HttpRequestParseResult::Incomplete;
  }

  if (buffer.substr(body_start, content_length_).size() > MAX_BODY_SIZE)
  {
    return HttpRequestParseResult::BodyTooLarge;
  }

  body_ = buffer.substr(body_start, content_length_);

  return HttpRequestParseResult::Complete;
}

HeaderKey HttpRequest::parse_header_key_(std::string_view key)
{
  if (to_lower_(key) == "host")
  {
    return HeaderKey::Host;
  }

  if (to_lower_(key) == "user-agent")
  {
    return HeaderKey::UserAgent;
  }

  if (to_lower_(key) == "accept")
  {
    return HeaderKey::Accept;
  }

  if (to_lower_(key) == "expect")
  {
    return HeaderKey::Expect;
  }

  if (to_lower_(key) == "content-length")
  {
    return HeaderKey::ContentLength;
  }

  if (to_lower_(key) == "content-type")
  {
    return HeaderKey::ContentType;
  }

  return HeaderKey::Unknown;
}

Expectation HttpRequest::parse_expectation_(std::string_view str)
{
  if (str == "100-continue")
  {
    return Expectation::Continue;
  }

  return Expectation::Unknown;
}

std::string HttpRequest::to_lower_(std::string_view str)
{
  std::string result(str);

  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c)
                 { return static_cast<char>(std::tolower(c)); });

  return result;
}

const HttpMethod&  HttpRequest::method() const { return method_; }
const std::string& HttpRequest::path() const { return path_; }
const std::string& HttpRequest::version() const { return version_; }
const std::string& HttpRequest::host() const { return host_; }
const std::string& HttpRequest::user_agent() const { return user_agent_; }
const std::string& HttpRequest::accept() const { return accept_; }
const Expectation& HttpRequest::expect() const { return expect_; }
const size_t& HttpRequest::content_length() const { return content_length_; }
const std::string& HttpRequest::content_type() const { return content_type_; }
const std::string& HttpRequest::body() const { return body_; }

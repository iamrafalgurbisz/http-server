#include "http/http-response.hpp"

#include <iostream>
#include <map>
#include <string>

#include "http/http-request.hpp"

HttpResponse::HttpResponse(HttpStatus status) : status_(status) {}

HttpResponse HttpResponse::Continue()
{
  return HttpResponse(HttpStatus::CONTINUE);
};
HttpResponse HttpResponse::Ok() { return HttpResponse(HttpStatus::OK); };
HttpResponse HttpResponse::OkNoContent()
{
  return HttpResponse(HttpStatus::OK_NO_CONTENT);
};
HttpResponse HttpResponse::BadRequest()
{
  return HttpResponse(HttpStatus::BAD_REQUEST);
};
HttpResponse HttpResponse::NotFound()
{
  return HttpResponse(HttpStatus::NOT_FOUND);
};
HttpResponse HttpResponse::MethodNotAllowed()
{
  return HttpResponse(HttpStatus::METHOD_NOT_ALLOWED);
};
HttpResponse HttpResponse::BodyTooLarge()
{
  return HttpResponse(HttpStatus::BODY_TOO_LARGE);
};
HttpResponse HttpResponse::UriTooLarge()
{
  return HttpResponse(HttpStatus::URI_TOO_LARGE);
};
HttpResponse HttpResponse::ExpectationFailed()
{
  return HttpResponse(HttpStatus::EXPECTATION_FAILED);
};
HttpResponse HttpResponse::HeadersTooLarge()
{
  return HttpResponse(HttpStatus::HEADERS_TOO_LARGE);
};
HttpResponse HttpResponse::InternalServerError()
{
  return HttpResponse(HttpStatus::INTERNAL_SERVER_ERROR);
};
HttpResponse HttpResponse::HttpVersionNotSupported()
{
  return HttpResponse(HttpStatus::HTTP_VERSION_NOT_SUPPORTED);
};

HttpResponse& HttpResponse::body(std::string_view body) &
{
  if (status_ == HttpStatus::CONTINUE)
  {
    std::cout << "Warning: HTTP 1xx responses cannot have body!\n";

    return *this;
  }

  if (status_ == HttpStatus::OK_NO_CONTENT)
  {
    std::cout << "Warning: HTTP 204 response cannot have body!\n";

    return *this;
  }

  body_ = body;

  return *this;
};

HttpResponse&& HttpResponse::body(std::string_view body) &&
{
  return std::move(this->body(body));
}

HttpResponse& HttpResponse::header(HeaderKey key, std::string_view value) &
{
  headers_.set(key, value);

  return *this;
}

HttpResponse&& HttpResponse::header(HeaderKey key, std::string_view value) &&
{
  return std::move(this->header(key, value));
}

void HttpResponse::clear_body()
{
  if (skip_body_)
  {
    std::cout << "Warning: body already skipped\n";
  }

  skip_body_ = true;
}

const std::string HttpResponse::get_serialized() const
{
  std::string res;

  res += "HTTP/1.1 ";
  res += get_status_line_();
  res += "\r\n";
  res += "Content-Type: text/plain; charset=utf-8\r\n";

  if (!body_.empty())
  {
    res += "Content-Length: " + std::to_string(body_.size()) + "\r\n";
  }
  else
  {
    res += "Content-Length: 0\r\n";
  }

  res += "Connection: close\r\n";

  std::map<HeaderKey, std::string_view>::iterator headers_it;

  for (auto headers_it = headers_.get().begin();
       headers_it != headers_.get().end(); headers_it++)
  {
    res += parse_header_key(headers_it->first) + ": " +
           std::string(headers_it->second) + "\r\n";
  }

  res += "\r\n";

  if (!body_.empty() && !skip_body_)
  {
    res += body_;
  }

  return res;
}

std::string HttpResponse::get_status_line_() const
{
  switch (status_)
  {
    case HttpStatus::CONTINUE:
      return std::to_string(static_cast<int>(status_)) + " Continue";
    case HttpStatus::OK:
      return std::to_string(static_cast<int>(status_)) + " OK";
    case HttpStatus::OK_NO_CONTENT:
      return std::to_string(static_cast<int>(status_)) + " No Content";
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
      return std::to_string(static_cast<int>(status_)) +
             " Internal Server Error";
    case HttpStatus::HTTP_VERSION_NOT_SUPPORTED:
      return std::to_string(static_cast<int>(status_)) +
             " HTTP Version Not Supported";
  }

  return std::to_string(static_cast<int>(HttpStatus::INTERNAL_SERVER_ERROR)) +
         "Internal Server Error";
};

bool HttpResponse::has_body() const { return !body_.empty(); }

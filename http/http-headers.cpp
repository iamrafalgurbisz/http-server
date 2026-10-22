#include "http-headers.hpp"

#include <iostream>
#include <optional>

HttpHeaders::HttpHeaders() {}

void HttpHeaders::set(HeaderKey key, std::string_view value)
{
  if (key == HeaderKey::ContentLength)
  {
    std::cout << "Passed Content-Length was ignored. Content-Length is being "
                 "automatically calculated from response's body\n";

    return;
  };

  auto found_header = headers_.find(key);

  if (found_header == headers_.end())
  {
    std::cout << "Warning: overwriting " << parse_header_key(key) << " with "
              << value << "\n";
  }

  headers_.insert({key, value});
}

const std::map<HeaderKey, std::string_view>& HttpHeaders::get() const
{
  return headers_;
}

std::optional<std::string_view> HttpHeaders::get(HeaderKey key) const
{
  auto found_header = headers_.find(key);

  if (found_header == headers_.end())
  {
    return std::nullopt;
  }

  return found_header->second;
}

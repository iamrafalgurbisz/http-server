#pragma once

#include <string>
#include <string_view>

enum class HttpMethodKey {
  Get,
  Post,
  Put,
  Patch,
  Delete,
  Head,
  Options,
  Unknown
};

class HttpMethod {
public:
  HttpMethod() = default;
  HttpMethod(std::string_view method_str);
  const std::string str() const;
  const bool is_valid() const;

private:
  const HttpMethodKey parse_key_(std::string_view method_str) const;
  std::string str_;
  HttpMethodKey key_;
  bool is_valid_;
};

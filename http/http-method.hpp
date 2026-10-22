#pragma once

#include <string>
#include <string_view>

enum class HttpMethodKey
{
  Get,
  Post,
  Put,
  Patch,
  Delete,
  Head,
  Options,
  Unknown
};

class HttpMethod
{
 public:
  HttpMethod() = default;
  HttpMethod(const std::string_view& method_str);
  HttpMethod(const HttpMethodKey& method_key);

  const std::string& str() const;
  HttpMethodKey      key() const;
  bool               is_valid() const;

 private:
  std::string   parse_key_(const HttpMethodKey& method_str) const;
  HttpMethodKey parse_str_(std::string_view method_str) const;
  std::string   str_;
  HttpMethodKey key_;
  bool          is_valid_;
};

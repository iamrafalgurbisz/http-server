#include "http/http-method.hpp"

HttpMethod::HttpMethod(const std::string_view& method_str)
{
  HttpMethodKey method_key = parse_str_(method_str);

  if (method_key == HttpMethodKey::Unknown)
  {
    is_valid_ = false;
  }
  else
  {
    is_valid_ = true;
  }

  key_ = method_key;
  str_ = method_str;
}

HttpMethod::HttpMethod(const HttpMethodKey& method_key)
{
  if (method_key == HttpMethodKey::Unknown)
  {
    is_valid_ = false;
  }
  else
  {
    is_valid_ = true;
  }

  key_ = method_key;
  str_ = parse_key_(method_key);
}

const std::string& HttpMethod::str() const { return str_; }
HttpMethodKey      HttpMethod::key() const { return key_; }

bool HttpMethod::is_valid() const { return is_valid_; }

HttpMethodKey HttpMethod::parse_str_(std::string_view method_str) const
{
  if (method_str == "GET")
  {
    return HttpMethodKey::Get;
  }

  if (method_str == "POST")
  {
    return HttpMethodKey::Post;
  }

  if (method_str == "PUT")
  {
    return HttpMethodKey::Put;
  }

  if (method_str == "PATCH")
  {
    return HttpMethodKey::Patch;
  }

  if (method_str == "DELETE")
  {
    return HttpMethodKey::Delete;
  }

  if (method_str == "HEAD")
  {
    return HttpMethodKey::Head;
  }

  if (method_str == "OPTIONS")
  {
    return HttpMethodKey::Options;
  }

  return HttpMethodKey::Unknown;
}

std::string HttpMethod::parse_key_(const HttpMethodKey& method_str) const
{
  if (method_str == HttpMethodKey::Get)
  {
    return "GET";
  }

  if (method_str == HttpMethodKey::Post)
  {
    return "POST";
  }

  if (method_str == HttpMethodKey::Put)
  {
    return "PUT";
  }

  if (method_str == HttpMethodKey::Patch)
  {
    return "PATCH";
  }

  if (method_str == HttpMethodKey::Delete)
  {
    return "DELETE";
  }

  if (method_str == HttpMethodKey::Head)
  {
    return "HEAD";
  }

  if (method_str == HttpMethodKey::Options)
  {
    return "OPTIONS";
  }

  return "UNKNOWN";
}

#include "http/http-method.hpp"

HttpMethod::HttpMethod(std::string_view method_str) {
  HttpMethodKey method_key = parse_key_(method_str);

  if (method_key == HttpMethodKey::Unknown) {
    is_valid_ = false;
  } else {
    is_valid_ = true;
  }

  key_ = method_key;
  str_ = method_str;
}

const std::string HttpMethod::str() const { return str_; }

const bool HttpMethod::is_valid() const {
  if (key_ == HttpMethodKey::Unknown) {
    return false;
  }

  return true;
}

const HttpMethodKey HttpMethod::parse_key_(std::string_view method_str) const {
  if (method_str == "GET") {
    return HttpMethodKey::Get;
  }

  if (method_str == "POST") {
    return HttpMethodKey::Post;
  }

  if (method_str == "PUT") {
    return HttpMethodKey::Put;
  }

  if (method_str == "PATCH") {
    return HttpMethodKey::Patch;
  }

  if (method_str == "DELETE") {
    return HttpMethodKey::Delete;
  }

  if (method_str == "HEAD") {
    return HttpMethodKey::Head;
  }

  if (method_str == "OPTIONS") {
    return HttpMethodKey::Options;
  }

  return HttpMethodKey::Unknown;
}

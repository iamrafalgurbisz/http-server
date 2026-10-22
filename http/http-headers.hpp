#include <map>
#include <optional>
#include <string>

enum class HeaderKey
{
  Host,
  UserAgent,
  Allow,
  Accept,
  ContentLength,
  ContentType,
  Expect,
  Unknown
};

constexpr std::string parse_header_key(HeaderKey key)
{
  switch (key)
  {
    case HeaderKey::Allow:
      return "Allow";
    case HeaderKey::Accept:
      return "Accept";
    case HeaderKey::Host:
      return "Host";
    case HeaderKey::UserAgent:
      return "User-Agent";
    case HeaderKey::ContentLength:
      return "Content-Length";
    case HeaderKey::ContentType:
      return "Content-Type";
    case HeaderKey::Expect:
      return "Expect";
    case HeaderKey::Unknown:
      return "Unknown";
    default:
      return "Unknown";
  };
};

class HttpHeaders
{
 public:
  HttpHeaders();
  const std::map<HeaderKey, std::string_view>& get() const;
  std::optional<std::string_view>              get(HeaderKey key) const;
  void                                         set(HeaderKey, std::string_view);

 private:
  std::map<HeaderKey, std::string_view> headers_;
};

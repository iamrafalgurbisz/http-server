#include <string_view>

constexpr std::string_view WS = " \t\n\r\f\v";

std::string_view trim(std::string_view s)
{
  auto start = s.find_first_not_of(WS);

  if (start == std::string_view::npos) return {};

  auto end = s.find_last_not_of(WS);

  return s.substr(start, end - start + 1);
}

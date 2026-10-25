#include "logger/logger.hpp"

#include <iostream>

namespace
{
constexpr const char* RESET  = "\033[0m";
constexpr const char* WHITE  = "\033[37m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* RED    = "\033[31m";
}  // namespace

Logger::LogStream::LogStream(const char* color) : color_(color) {}

Logger::LogStream::~LogStream() { Logger::write(buffer_.str(), color_); }

Logger::LogStream Logger::info() { return LogStream{WHITE}; }
Logger::LogStream Logger::warning() { return LogStream{YELLOW}; }
Logger::LogStream Logger::error() { return LogStream{RED}; }

void Logger::write(const std::string& message, const char* color)
{
  std::cout << color << message << RESET << '\n';
}

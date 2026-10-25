#pragma once

#include <sstream>

class Logger
{
 public:
  class LogStream
  {
   public:
    explicit LogStream(const char* color);
    ~LogStream();

    template <typename T>
    LogStream& operator<<(const T& value)
    {
      buffer_ << value;
      return *this;
    }

   private:
    std::ostringstream buffer_;
    const char*        color_;
  };

  static LogStream info();
  static LogStream warning();
  static LogStream error();

 private:
  static void write(const std::string& message, const char* color);
};

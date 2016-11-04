#pragma once

#include <functional>

#include "logger/Message.hpp"

namespace logger
{

typedef std::function < std::string(const Message&) > Formatter;

class StandardFormatter
{
public:
  std::string operator()(const Message& message) const
  {
    return message.content;
  }
};

} // logger
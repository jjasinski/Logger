#pragma once

#include <string>
#include <sstream>

namespace logger
{

  template<typename ... Args>
  std::string string_format(const std::string& format, Args ... args)
  {
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // extra place for '\0'

    std::string result(size, '\0');
    //result.resize(size);
    snprintf(&result[0], size, format.c_str(), args ...); // generate string with '\0' at the end
    result.resize(size - 1); // cut the last '\0' character
    return std::move(result);
  }

  std::string toString(const std::thread::id& id)
  {
    std::ostringstream buffer;
    buffer << id;
    return buffer.str();

  }

} // logger
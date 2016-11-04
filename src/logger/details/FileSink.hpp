#pragma once

#include <atomic>
#include <fstream>

#include "logger/Sink.hpp"
#include "logger/Formatter.hpp"

namespace logger
{
namespace details
{

class FileSink : public Sink
{
public:
  explicit FileSink(const std::string& name, Formatter _formatter)
    :
    formatter(_formatter)
  {
    file.open(name, std::ofstream::out/* | std::ofstream::app*/);
  }

  virtual void send(std::unique_ptr<Message> message) override
  {
    file << formatter(*message);// << std::endl;
  }

  virtual void flush() override
  {
    file.flush();
  }
private:
  std::ofstream file;
  Formatter formatter;
};

} // details
} // logger
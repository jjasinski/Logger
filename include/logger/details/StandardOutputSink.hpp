#pragma once

#include <atomic>

#include "logger/Sink.hpp"
#include "logger/Formatter.hpp"
#include "logger/BinaryFlag.hpp"

namespace logger
{
namespace details
{

template<typename BinaryFlag>
class StandardOutputSink : public Sink
{
public:
  explicit StandardOutputSink(Formatter _formatter)
    :
    formatter(_formatter),
    errNeedsFlush(false),
    outNeedsFlush(false)
  {

  }

  virtual void send(std::unique_ptr<Message> message) override
  {
    if (message->level >= Level::WARNING)
    {
      std::cerr << formatter(*message) << std::endl;
      errNeedsFlush = true;
    }
    else
    {
      std::cout << formatter(*message);// << std::endl;
      outNeedsFlush = true;
    }
  }

  virtual void flush() override
  {
    if (errNeedsFlush.exchange(false))
    {
      std::cerr.flush();
    }
    if (outNeedsFlush.exchange(false))
    {
      std::cout.flush();
    }
  }
private:
  Formatter formatter;
  BinaryFlag errNeedsFlush;
  BinaryFlag outNeedsFlush;
};

} // details
} // logger
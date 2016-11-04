#pragma once

#include <atomic>
#include <functional>

#include "logger/Sink.hpp"
#include "logger/Formatter.hpp"

namespace logger
{

class SinkFactory
{
public:
  typedef std::shared_ptr< Sink > SinkPtr;

  SinkFactory() = default;
  virtual ~SinkFactory() = default;

  virtual SinkPtr createStandardOutputSink(Formatter formatter) = 0;

  virtual SinkPtr createFileSink(const std::string& name, Formatter formatter) = 0;
};

} // logger
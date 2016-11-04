#pragma once

#include <memory>

#include "logger/SinkFactory.hpp"
#include "logger/Formatter.hpp"

#include "logger/details/StandardOutputSink.hpp"
#include "logger/details/FileSink.hpp"
#include "logger/details/MultithreadSink.hpp"

namespace logger
{
namespace details
{

class MultithreadSinkFactory : public SinkFactory
{
public:
  typedef std::shared_ptr< Sink > SinkPtr;

  virtual SinkPtr createStandardOutputSink(Formatter formatter)
  {
    auto internalSink = std::make_shared< StandardOutputSink< AtomicFlag > >(formatter);
    return makeMultithreadSink(internalSink);
  }

  virtual SinkPtr createFileSink(const std::string& name, Formatter formatter)
  {
    auto internalSink = std::make_shared< FileSink >(name, formatter);
    return makeMultithreadSink(internalSink);
  }

private:
  SinkPtr makeMultithreadSink(SinkPtr internalSink)
  {
    return std::make_shared< MultithreadSink >(internalSink);
  }
};

} // details
} // logger
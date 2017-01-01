#pragma once

#include <memory>

#include "logger/SinkFactory.hpp"
#include "logger/Formatter.hpp"

#include "logger/details/StandardOutputSink.hpp"
#include "logger/details/FileSink.hpp"

#ifdef LOGGER_USE_MOODYCAMEL_CONCURRENT_QUEUE
#include "logger/details/ConcurrentQueueSink.hpp"
#else
#include "logger/details/MultithreadSink.hpp"
#endif

namespace logger
{
namespace details
{

class MultithreadSinkFactory : public SinkFactory
{
public:
  typedef std::shared_ptr< Sink > SinkPtr;

  
#ifdef LOGGER_USE_MOODYCAMEL_CONCURRENT_QUEUE
  typedef ConcurrentQueueSink DefinedMultitherdSink;
#else
   typedef MultithreadSink DefinedMultitherdSink;
#endif

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
#ifdef LOGGER_USE_MOODYCAMEL_CONCURRENT_QUEUE
    return std::make_shared< ConcurrentQueueSink >(internalSink);
#else
    return std::make_shared< MultithreadSink >(internalSink);
#endif
    //
  }
};

} // details
} // logger
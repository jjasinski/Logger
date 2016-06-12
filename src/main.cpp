#include <memory>
#include <chrono>
#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include <vector>
#include <mutex>
#include <algorithm>

#include <ctime>
#include <thread>
#include <assert.h>

#include "logger/Message.hpp"
#include "logger/Sink.hpp"
#include "logger/Logger.hpp"
#include "logger/Registry.hpp"

#include "logger/impl/MultithreadRegistry.hpp"

// TODO
// add posibility of flushing from the same line like:
// logger.debug(...).flush();

using namespace logger;

//class Formatter // assigned to sink! (optional)
//{
//public:
//  virtual ~Formatter() = 0;
//  virtual std::string format(const Message& message) const = 0;
//};

class SimpleStandardOutputSink: public Sink
{
public:
  SimpleStandardOutputSink()
    :
    begin(DefaultClock::now())
  {

  }

  // Inherited via Sink
  virtual void send(std::unique_ptr<Message> message) override
  {
    auto duration = message->time - begin;
    auto ms = duration.count() / 1000;// / (10 >> 3);
    if (message->level >= Level::WARNING)
    {
      std::cerr << ms << ": " << message->content << std::endl;
    }
    else
    {
      std::cout << ms  << ": " << message->content << std::endl;
    }
  }

  virtual void flush() override
  {
    //
  }
  const DefaultClock::time_point begin;
  //std::atomic_bool 
};

//class SinkFactory
//{
//public:
//  SinkFactory() = default;
//  virtual ~SinkFactory() = default;
//
//  typedef std::shared_ptr< Sink > SinkPtr;
//
//  virtual SinkPtr createStandardOutputSink() = 0;
//};

// TODO add NullFlag with empty functions?

struct AtomicFlag
{
  AtomicFlag(bool flag)
    :
    value(flag)
  {
  }

  bool operator=(bool flag)
  {
    return value = flag;
  }

  operator bool() const
  {
    return value;
  }

  bool exchange(bool flag)
  {
    return value.exchange(flag);
  }

  std::atomic_bool value;
};

struct NotAtomicFlag
{
  NotAtomicFlag(bool flag)
    :
    value(flag)
  {
  }

  bool operator=(bool flag)
  {
    return value = flag;
  }

  operator bool() const
  {
    return value;
  }

  bool exchange(bool flag)
  {
    bool res = value;
    value = flag;
    return res;
  }

  bool value;
};


typedef std::function < std::string(const Message&) > Formatter;
template</*typename Formatter, */typename BinaryFlag>
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
      std::cout << formatter(*message) << std::endl;
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

class StandardFormatter
{
public:
  std::string operator()(const Message& message) const
  {
    return message.content;
  }
};

class SinkFactory
{
public:
  typedef std::shared_ptr< Sink > SinkPtr;

  SinkFactory() = default;
  virtual ~SinkFactory() = default;

  virtual SinkPtr createStandardOutputSink(Formatter formatter) = 0;
};

typedef std::shared_ptr< Sink > SinkPtr;

class MultithreadSink : public Sink
{
public:
  explicit MultithreadSink(std::shared_ptr< Sink > aSink)
    :
    internalSink(aSink)
  {
    messages.reserve(10000);//10K
  }

  virtual void send(std::unique_ptr<Message> message) override
  {
    std::lock_guard< std::mutex > lock(mt);
    messages.push_back(std::move(message));
  }
 
  virtual void flush() override
  {
    mt.lock();
    auto localMessages = std::move(messages);
    mt.unlock();

    if (localMessages.empty())
    {
      return;
    }
   // consume queue by target Sink
   for (auto it = localMessages.begin(); it != localMessages.end(); ++it)
    {
      internalSink->send(std::move(*it));
    }
    
    internalSink->flush();
  }
private:
  std::shared_ptr< Sink > internalSink;

  std::mutex mt;
  std::vector< std::unique_ptr<Message> > messages;
};

class MultthreadSinkFactory : public SinkFactory
{
public:
  typedef std::shared_ptr< Sink > SinkPtr;

  virtual SinkPtr createStandardOutputSink(Formatter formatter)
  {
    auto internalSink = std::make_shared< StandardOutputSink< AtomicFlag > >(formatter);
    return makeMultithreadSink(internalSink);
  }

private:
  SinkPtr makeMultithreadSink(SinkPtr internalSink)
  {
    return std::make_shared< MultithreadSink >(internalSink);
  }
};

/*
// register general logger mechanism (multithreading or not)
logger::registry().registerHandle(...);

logger::registry()->get()->debug(...);
logger::registry()->get("name")->debug(...);
auto logger = logger::registry()->get();
logger->debug(...);
logger->info(...).flush();

// release all logger resources
logger::registry().releaseHandle();
*/

namespace logger
{

} // logger

void main()
{
  auto factory = std::make_unique< MultthreadSinkFactory >();
  
  registry().registerHandle( std::make_unique< MultithreadRegistryHandle >() );


  auto sink = factory->createStandardOutputSink(
    [] (const Message& message)
  {
    return "[" + message.loggerContext->name + "] " + message.content;
  }
  );
 
  const std::string DEFAULT_LOGGER_NAME = "module name";

  auto logger = std::make_shared< Logger >(DEFAULT_LOGGER_NAME);
  //logger->sink = sink;
  
  registry()->registerLogger(logger);

  logger->debug(LOGGER_CALL_CONTEXT, "message 1");
  
  logger->filteringLevel = Level::DEBUG;
  auto begin = DefaultClock::now();
  const auto THOUSAND = 1000;
  const auto MILLION = THOUSAND * THOUSAND;
  //logger->filteringLevel = Level::NEVER;

  for (int i = 0; i < MILLION; ++i)
  {
    //logger->debug(LOGGER_CALL_CONTEXT, "");
    //logger->debug(LOGGER_CALL_CONTEXT, "message...");
    //logger->debug(LOGGER_CALL_CONTEXT, "message..." + std::to_string(i));
    
    registry()->getLogger(DEFAULT_LOGGER_NAME)->debug(LOGGER_CALL_CONTEXT, "message...");
    //registry()->getLogger(DEFAULT_LOGGER_NAME)->debug(LOGGER_CALL_CONTEXT, "message..." + std::to_string(i));
    
    /*logger.debug(LOGGER_CALL_CONTEXT, "message A: " + std::to_string(i) + "/" + std::to_string(MILLION));*/
    //logger.critical(LOGGER_CALL_CONTEXT, "Error");
    //logger.critical(LOGGER_CALL_CONTEXT, "Critical error");
  }
  //doBreak = true;
  auto end = DefaultClock::now();

  auto duration = end - begin;
  std::cout << "total time: " << duration.count() * 1. / 1000000000. << "sec" << std::endl;

  //logger->flush();
  //logger.debug(LOGGER_CALL_CONTEXT, "message 2");
  //logger.debug(LOGGER_CALL_CONTEXT, "message 3");
  //logger.debug(LOGGER_CALL_CONTEXT, "message 4");

  //logger.flush();

  registry().unregisterHandle();
}
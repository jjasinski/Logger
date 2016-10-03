#include <memory>
#include <chrono>
#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include <vector>
#include <mutex>
#include <algorithm>
#include <iterator>

#include <ctime>
#include <thread>
#include <assert.h>
#include <fstream>

#include "logger/Message.hpp"
#include "logger/Sink.hpp"
#include "logger/Logger.hpp"
#include "logger/Registry.hpp"
#include "logger/StringHelpers.hpp"

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
    //auto msg = formatter(*message);
    //file.write(msg.c_str(), msg.size());
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

  virtual SinkPtr createFileSink(const std::string& name, Formatter formatter) = 0;
};

typedef std::shared_ptr< Sink > SinkPtr;


class MultithreadSink : public Sink
{
public:
  explicit MultithreadSink(std::shared_ptr< Sink > aSink)
    :
    internalSink(aSink)
  {
  }

  virtual void send(std::unique_ptr<Message> message) override
  {
    std::lock_guard< std::mutex > lock(mt);
    messages.push_back(std::move(message));
  }
 
  virtual void flush() override
  {
    std::lock_guard< std::mutex > lock(flushMt);

    mt.lock();
    auto messagesBuffer = std::move(messages);
    assert( messages.empty() );
    mt.unlock();

   // consume queue by target Sink
   for (auto it = messagesBuffer.begin(); it != messagesBuffer.end(); ++it)
    {
      internalSink->send(std::move(*it));
    }
    
   internalSink->flush();
  }

private:
  std::shared_ptr< Sink > internalSink;

  std::mutex flushMt; // only one thread can be inside flush() method at the time

  std::mutex mt;
  std::vector< std::unique_ptr<Message> > messages;
};

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


void main()
{
  auto factory = std::make_unique< MultithreadSinkFactory >();
  
  registry().registerHandle( std::make_unique< MultithreadRegistryHandle >() );

  Formatter formatter = 
    [](const Message& message)
  {
    //static long long prev
    auto ns = message.time.time_since_epoch().count();
    return string_format("%lld [%s] {%s, %s:%i} %s\n",
      ns / 1000, // nanosec to microsec
      message.loggerContext->name.c_str(),
      toString(message.threadId).c_str(),
      message.callContext.function,
      message.callContext.line,
      message.content.c_str()
    );
  };

  long long lp = 1;
  Formatter csvFormatter =
    [&lp](const Message& message)
  {
    //static long long prev
    auto ns = message.time.time_since_epoch().count();
    return string_format("%lld,%lld,%s\n",
      lp++,
      ns,
      message.content.c_str()
    );
  };

  auto consoleSink = factory->createStandardOutputSink(formatter);
  auto fileSink = factory->createFileSink("test.log", formatter);
  //auto csvSink = factory->createFileSink("out.csv", csvFormatter);
 
  const std::string DEFAULT_LOGGER_NAME = "module name";

  //csvSink = std::make_shared< FileSink >("out.csv", csvFormatter);

  auto logger = std::make_shared< Logger >(DEFAULT_LOGGER_NAME);
  logger->sink = consoleSink;
  logger->sink = fileSink;
  //logger->sink = csvSink;
  
  registry()->registerLogger(logger);

  logger->debug(LOGGER_CALL_CONTEXT, "message 1");
  
  logger->filteringLevel = Level::DEBUG;
  auto begin = DefaultClock::now();
  const auto THOUSAND = 1000;
  const auto MILLION = THOUSAND * THOUSAND;
  logger->filteringLevel = Level::NEVER;

  const auto ITERATIONS = MILLION * 5;

  auto threadWorker = [&]()
  {
    for (int i = 0; i < ITERATIONS; ++i)
    {
      //logger->debug(LOGGER_CALL_CONTEXT, "message... itertion #" + std::to_string(i));

      logger->debug(LOGGER_CALL_CONTEXT, [&]()->std::string
      {
        return "message... itertion #" + std::to_string(i);
      }
      );
    }
  };

  auto THREAD_COUNT = 4;
  std::vector< std::thread > threads;
#if 0
  std::generate_n(std::back_inserter(threads), THREAD_COUNT,
    [&]()
  {
    return std::thread(threadWorker);
  }
  );
#else
  threadWorker();
#endif


  //for (int i = 0; i < THREAD_COUNT; i++)
  //{
  //  std::thread t(callback);
  //  threads.push_back( std::move(t) );
  //}

  std::for_each(threads.begin(), threads.end(), 
    [](std::thread& t)
  {
    t.join();
  }
  );


  //logger->flush();

  auto end = DefaultClock::now();

  auto duration = end - begin;
  

  std::cout << "total time: " << duration.count() * 1. / 1000000000. << "sec" << std::endl;
  std::cout << "mean time: " << duration.count() * 1. / ITERATIONS << "nano sec" << std::endl;

  //logger->flush();

  registry().unregisterHandle();
}
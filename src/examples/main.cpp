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
#include <queue>

#include <ctime>
#include <thread>
#include <assert.h>
#include <fstream>

#define LOGGER_USE_MOODYCAMEL_CONCURRENT_QUEUE 1
//#undef LOGGER_USE_MOODYCAMEL_CONCURRENT_QUEUE

#include "logger/Message.hpp"
#include "logger/Sink.hpp"
#include "logger/Logger.hpp"
#include "logger/Registry.hpp"
#include "logger/StringHelpers.hpp"

#include "logger/BinaryFlag.hpp"
#include "logger/SinkFactory.hpp"

#include "logger/details/MultithreadRegistry.hpp"

//#include <concurrent_queue.h>

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
  //auto factory = std::make_unique< details::MultithreadSinkFactory >();

  registry().registerHandle(std::make_unique< details::MultithreadRegistryHandle >());

  Formatter formatter =
    [](const Message& message)
  {
    //auto logContext = message.loggerContext.lock();
    //static long long prev
    auto ns = message.time.time_since_epoch().count();
    return string_format("%lld [%s] {%s, %s:%i} %s\n",
      ns / 1000, // nanosec to microsec
      message.loggerContext.lock()->name.c_str(),
      //message.loggerName.c_str(),
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

  auto consoleSink = registry()->getSinkFactory()->createStandardOutputSink(formatter);
  auto fileSink = registry()->getSinkFactory()->createFileSink("test.log", formatter);
  //auto csvSink = factory->createFileSink("out.csv", csvFormatter);

  const std::string DEFAULT_LOGGER_NAME = "module name";

  //csvSink = std::make_shared< FileSink >("out.csv", csvFormatter);

  auto logger = std::make_shared< Logger >(DEFAULT_LOGGER_NAME);
  //logger->sink = consoleSink;
  logger->sink = fileSink;
  //logger->sink = csvSink;

  registry()->registerLogger(logger);

  logger->debug(LOGGER_CALL_CONTEXT, "message 1");

  logger->filteringLevel = Level::DEBUG;
  auto begin = DefaultClock::now();
  const auto THOUSAND = 1000;
  const auto MILLION = THOUSAND * THOUSAND;
  logger->filteringLevel = Level::DEBUG;

  const auto ITERATIONS = 100 * THOUSAND;//MILLION;

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
#if 1
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


  //

  auto end = DefaultClock::now();
  auto duration = end - begin;


  std::cout << "sending time (only register): " << duration.count() * 1. / 1000000000. << "sec" << std::endl;
  std::cout << "[mean time: " << duration.count() * 1. / ITERATIONS << "nano sec]" << std::endl;


  logger->flush();
  auto finished = DefaultClock::now();
  auto fullDuration = finished - begin;

  std::cout << "full time (register and writing): " << fullDuration.count() * 1. / 1000000000. << "sec" << std::endl;
  std::cout << "[mean time: " << fullDuration.count() * 1. / ITERATIONS << "nano sec]" << std::endl;

  logger->flush();

  registry().unregisterHandle();
}
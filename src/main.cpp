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
#include <shared_mutex>

#include "logger/Message.hpp"
#include "logger/Sink.hpp"
#include "logger/Logger.hpp"

// TODO
// add posibility of flushing from the same line like:
// logger.debug(...).flush();

using namespace logger;

class Formatter // assigned to sink! (optional)
{
public:
  virtual ~Formatter() = 0;
  virtual std::string format(const Message& message) const = 0;
};

class StandardOutputSink: public Sink
{
public:
  StandardOutputSink()
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

class Lock
{

};
std::atomic_bool doBreak = false;

void main()
{
  auto sink = std::make_shared< StandardOutputSink >();
 
  Logger logger("");
  logger.sink = std::make_shared< MultithreadSink >(sink);
  //logger.sink = sink;

  std::thread thread(
    [&logger]()
  {
    while (!doBreak)
    {
      logger.flush();
      std::this_thread::yield();
    }
    
  }
  );

  //thread.

  logger.debug(LOGGER_CALL_CONTEXT, "message 1");

  logger.filteringLevel = Level::DEBUG;
  auto begin = DefaultClock::now();
  const auto MILLION = 1000000;
  const auto THOUSAND = 1000;
  for (int i = 0; i < THOUSAND; ++i)
  {
    logger.debug(LOGGER_CALL_CONTEXT, "message...");
    /*logger.debug(LOGGER_CALL_CONTEXT, "message A: " + std::to_string(i) + "/" + std::to_string(MILLION));*/
    logger.critical(LOGGER_CALL_CONTEXT, "Error");
    logger.critical(LOGGER_CALL_CONTEXT, "Critical error");
  }
  doBreak = true;
  auto end = DefaultClock::now();
  thread.join();

  auto duration = end - begin;
  std::cout << "total time: " << duration.count() * 1. / 1000000000. << "sec" << std::endl;
  //logger.debug(LOGGER_CALL_CONTEXT, "message 2");
  //logger.debug(LOGGER_CALL_CONTEXT, "message 3");
  //logger.debug(LOGGER_CALL_CONTEXT, "message 4");

  //logger.flush();
}
#pragma once;

#include <shared_mutex>
#include <map>
#include <unordered_map>

#include "logger/Registry.hpp"

#include "logger/details/MultithreadSinkFactory.hpp"

namespace logger
{

class MultithreadRegistryHandle : public RegistryHandle
{
private:
  typedef std::map< std::string, LoggerPtr > LoggersMap;
  //typedef std::unordered_map< std::string, LoggerPtr > LoggersMap;
public:
  MultithreadRegistryHandle()
  {
    doBreak = false;
    flushingThread = makeFlushingThread();
  }

  virtual ~MultithreadRegistryHandle()
  {
    doBreak = true;
    try
    {
      flushingThread.join();
    }
    catch (...)
    {
      assert(false);
    }
  }

  virtual LoggerPtr getLogger(const std::string& name)
  {
#if 1
    thread_local LoggerPtr lastResult = nullptr;
    if (lastResult)
    {
      if (lastResult->getName() == name)
      {
        return lastResult;
      }
    }
#endif
    LoggerPtr result;
    mutex.lock_shared();
    auto findIt = loggers.find(name);
    if (findIt != loggers.end())
    {
      result = findIt->second;
    }
    mutex.unlock_shared();

    if (!result)
    {
      throw std::runtime_error("Logger not found");
    }
#if 1
    lastResult = result;
#endif
    return result;
  }
  virtual LoggerPtr unregisterLogger(const std::string& name)
  {
    LoggerPtr result;
    mutex.lock();
    if (loggers.count(name))
    {
      result = loggers[name];
      loggers.erase(name);
    }
    mutex.unlock();
    return result;
  }

  virtual void registerLogger(LoggerPtr logger)
  {
    assert(logger);
    mutex.lock();
    const auto& name = logger->getName();
    if (loggers.count(name))
    {
      throw std::runtime_error("Logger already register");
    }
    loggers[name] = logger;
    mutex.unlock();
  }

  virtual std::shared_ptr< SinkFactory > getSinkFactory()
  {
    return std::make_shared< details::MultithreadSinkFactory >();
  }
private:
  std::shared_timed_mutex mutex;
  LoggersMap loggers;
  std::atomic_bool doBreak;
  std::thread flushingThread;

  void flushingWork()
  {
#if 0
    std::vector< LoggerPtr > loggersList;
    loggersList.reserve(loggers.size());
    mutex.lock_shared();
    for (auto it = loggers.begin(); it != loggers.end(); ++it)
    {
      loggersList.push_back(it->second);
    }
    mutex.unlock_shared();


    for (auto it = loggersList.begin(); it != loggersList.end(); ++it)
    {
      (*it)->flush();
    }
    std::this_thread::yield();
#endif
#if 1
    mutex.lock_shared();
    for (auto it = loggers.begin(); it != loggers.end(); ++it)
    {
      auto logger = it->second;
      logger->flush(); // flushing inside a CS may be not a good idea...
    }
    mutex.unlock_shared();
    std::this_thread::yield();
#endif
}

  std::thread makeFlushingThread()
  {
    std::thread thread(
      [this]()
    {
      while (!doBreak)
      {
        flushingWork();
      }

    }
    );

    return std::move(thread);
  }

};

} // logger
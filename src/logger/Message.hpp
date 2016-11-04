#pragma once

#include <memory>
#include <chrono>
#include <string>
#include <thread>

#ifdef _MSC_VER
#define LOGGER_CALL_CONTEXT CallContext(__FUNCTION__, __FUNCSIG__, __FILE__, __LINE__)
#else
#define LOGGER_CALL_INFO SourceCodeContext(__FUNCTION__, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

namespace logger
{


/** source code specyfic information
 */
struct CallContext
{
  CallContext(const char* aFunc, const char* aDecorated, const char* aFile, unsigned int aLine)
    :
    function(aFunc),
    decoratedFunction(aDecorated),
    file(aFile),
    line(aLine)
  {
  }

  CallContext(const CallContext& info) = default;

  const char* function;
  const char* decoratedFunction;
  const char* file;
  const unsigned int line;
};

/* logger specyfic information
*/
struct LoggerContext
{
  explicit LoggerContext(const std::string& aName)
    :
    name(aName)
  {
  }

  const std::string name;
};

typedef std::chrono::high_resolution_clock DefaultClock;

enum class Level
{
  TRACE = 0,
  DEBUG_FINEST,
  DEBUG_FINER,
  DEBUG_FINE,
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  CRITICAL,
  NEVER
};

struct Message
{
  explicit Message(const CallContext& aCall, std::shared_ptr<const LoggerContext> aLogger)
    :
    callContext(aCall),
    loggerContext(aLogger),
    time(DefaultClock::now()),
    threadId(std::this_thread::get_id())
  {
  }

  // general information
  CallContext callContext;
  std::shared_ptr<const LoggerContext> loggerContext;

  Level level;
  std::string content;

  // additional information
  std::chrono::time_point< DefaultClock > time;
  std::thread::id threadId;
};

}// end logger
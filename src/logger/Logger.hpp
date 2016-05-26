#pragma once

#include "logger/Sink.hpp"

namespace logger
{

  class Logger
  {
  public:
    explicit Logger(const std::string& aName)
      :
      loggerContext(std::make_shared< LoggerContext >(aName)),
      filteringLevel(Level::NONE),
      autoFlushLevel(Level::NONE),
      sink(std::make_shared< NullSink >())
    {
    }

    Level filteringLevel;
    Level autoFlushLevel;
    std::shared_ptr< Sink > sink;

    void debug(const CallContext& aContext, std::string&& aMsg)
    {
      return log(aContext, Level::DEBUG, std::move(aMsg));
    }

    void flush()
    {
      sink->flush();
    }

  private:
    std::shared_ptr<const LoggerContext> loggerContext;

    void log(const CallContext& aContext, Level aLevel, std::string&& aMsg)
    {
      if (aLevel >= filteringLevel)
      {
        auto message = makeMessage(aContext, aLevel);
        message->raw = std::move(aMsg);
        sink->send(std::move(message));
        if (aLevel >= autoFlushLevel)
        {
          sink->flush();
        }
      }
    }

    std::unique_ptr< Message > makeMessage(const CallContext& aContext, Level aLevel)
    {
      auto message = std::make_unique< Message >(aContext, loggerContext);
      message->level = aLevel;
      return std::move(message);
    }
  };

} // end logger

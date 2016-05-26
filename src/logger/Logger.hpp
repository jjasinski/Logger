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
      filteringLevel(Level::NEVER),
      autoFlushLevel(Level::NEVER),
      sink(std::make_shared< NullSink >())
    {
    }

    Level filteringLevel;
    Level autoFlushLevel;
    std::shared_ptr< Sink > sink;

    void critical(const CallContext& aContext, std::string&& aMsg)
    {
      return log(aContext, Level::CRITICAL, std::move(aMsg));
    }

    void error(const CallContext& aContext, std::string&& aMsg)
    {
      return log(aContext, Level::ERROR, std::move(aMsg));
    }

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
        auto message = makeMessage(aContext);
        message->level = aLevel;
        message->content = std::move(aMsg);

        sink->send(std::move(message));
        if (aLevel >= autoFlushLevel)
        {
          sink->flush();
        }
      }
    }

    std::unique_ptr< Message > makeMessage(const CallContext& aContext)
    {
      return std::make_unique< Message >(aContext, loggerContext);
    }
  };

} // end logger

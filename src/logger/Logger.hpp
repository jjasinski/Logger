#pragma once

#include "logger/Sink.hpp"

namespace logger
{

  class Logger
  {
  public:
    typedef std::function< std::string() > MakeMessageCallback;

    explicit Logger(const std::string& name)
      :
      loggerContext(std::make_shared< LoggerContext >(name)),
      filteringLevel(Level::NEVER),
      autoFlushLevel(Level::NEVER),
      sink(std::make_shared< NullSink >())
    {
    }

    std::atomic< Level > filteringLevel;
    std::atomic< Level > autoFlushLevel;
    std::shared_ptr< Sink > sink;

    /* Simple versions */
    void critical(const CallContext& aContext, std::string&& message)
    {
      return log(aContext, Level::CRITICAL, std::move(message));
    }

    void error(const CallContext& aContext, std::string&& message)
    {
      return log(aContext, Level::ERROR, std::move(message));
    }

    void debug(const CallContext& context, std::string&& message)
    {
      return log(context, Level::DEBUG, std::move(message));
    }

    /* Lazy Message Formation versions */
    void debug(const CallContext& context, MakeMessageCallback messgeCallback)
    {
      return log(context, Level::DEBUG, messgeCallback);
    }

    void flush()
    {
      sink->flush();
    }

    const std::string& getName() const
    {
      return loggerContext->name;
    }

  private:
    std::shared_ptr<const LoggerContext> loggerContext;

    void log(const CallContext& context, Level level, std::string&& content)
    {
      if (level >= filteringLevel)
      {
        auto message = makeMessage(context);
        message->level = level;
        message->content = std::move(content);

        sink->send(std::move(message));
        autoFlushIfNeeded(level);
      }
    }

    void log(const CallContext& context, Level level, MakeMessageCallback messgeCallback)
    {
      if (level >= filteringLevel)
      {
        auto message = makeMessage(context);
        message->level = level;
        message->content = messgeCallback();

        sink->send(std::move(message));
        autoFlushIfNeeded(level);
      }
    }

    std::unique_ptr< Message > makeMessage(const CallContext& context)
    {
      return std::make_unique< Message >(context, loggerContext);
    }

    void autoFlushIfNeeded(Level level)
    {
      if (level >= autoFlushLevel)
      {
        sink->flush();
      }
    }


  };

} // end logger

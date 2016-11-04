#pragma once

#include "logger/Message.hpp"

namespace logger
{

class Sink
{
public:
  Sink() = default;
  virtual ~Sink() = default;

  virtual void send(std::unique_ptr< Message > message) = 0;
  virtual void flush() = 0;
};

class NullSink : public Sink
{
public:
  NullSink() = default;
  virtual ~NullSink() = default;

  virtual void send(std::unique_ptr< Message > message)
  {
    // empty implementation
  }

  virtual void flush()
  {
    // empty implementation
  }
};

}// logger
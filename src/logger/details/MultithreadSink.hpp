#pragma once

#include <atomic>
#include <mutex>
#include <vector>
#include <algorithm>
#include <cassert>

#include "logger/Sink.hpp"
#include "logger/Formatter.hpp"

namespace logger
{
namespace details
{

/* naive multithread Sink implementatin
 * will be upgrade :)
 */
class MultithreadSink : public Sink
{
public:
  typedef std::vector< std::unique_ptr<Message> > Messages;

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

    auto buffer = extractMessages();
    std::for_each(buffer.begin(), buffer.end(),
      [&](auto&& message)
    {
      internalSink->send(std::move(message));
    }
    );

    internalSink->flush();
  }

private:
  std::shared_ptr< Sink > internalSink;

  std::mutex flushMt; // only one thread can be inside flush() method at the time

  std::mutex mt;
  Messages messages;

  Messages extractMessages()
  {
    std::lock_guard< std::mutex > lock(mt);
    auto buffer = std::move(messages);
    assert(messages.empty());
    return std::move(buffer);
  }
};

} // details
} // logger
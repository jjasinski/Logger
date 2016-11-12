#pragma once

#include <atomic>

#include "logger/Message.hpp"

namespace logger
{

// TODO add NullFlag with empty functions?

/** Flag interfaces
 */

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

} // logger
#pragma once;

#include "logger/Logger.hpp"
#include <cassert>

namespace logger
{

  typedef std::shared_ptr< Logger > LoggerPtr;

  class RegistryHandle
  {
  public:
    virtual ~RegistryHandle() = default;

    virtual LoggerPtr getLogger(const std::string& name) = 0;
    virtual LoggerPtr unregisterLogger(const std::string& name) = 0;
    virtual void registerLogger(LoggerPtr logger) = 0;
  };


  class Registry;

  /* returns the global registry instance
   */
  Registry& registry();

  class Registry
  {
  public:

    friend Registry& registry()
    {
      static Registry instance;
      return instance;
    }

    void registerHandle(std::unique_ptr< RegistryHandle > aHandle)
    {
      handle = std::move(aHandle);
    }

    std::unique_ptr< RegistryHandle > unregisterHandle()
    {
      return std::move(handle);
    }

    RegistryHandle* operator->()
    {
      assert(handle); // TODO exception may be better
      return handle.get();
    }

  private:
    /* private default constructor
     * can be calling by registry() function
     */
    Registry() = default;

    std::unique_ptr< RegistryHandle > handle;
  };

}// end logger
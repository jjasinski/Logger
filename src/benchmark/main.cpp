#include <memory>
#include <chrono>
#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include <vector>
#include <mutex>
#include <algorithm>
#include <iterator>
#include <queue>

#include <ctime>
#include <thread>
#include <assert.h>
#include <fstream>

#include "logger/Message.hpp"
#include "logger/Sink.hpp"
#include "logger/Logger.hpp"
#include "logger/Registry.hpp"
#include "logger/StringHelpers.hpp"

#include "logger/BinaryFlag.hpp"
#include "logger/SinkFactory.hpp"

#include "logger/details/MultithreadRegistry.hpp"

/*
  implementation inspired by NanoLog benchmark: https://github.com/Iyengar111/NanoLog/blob/master/nano_vs_spdlog_vs_g3log_vs_reckless.cpp
*/

const auto THOUSAND = 1000;
const auto MILLION = THOUSAND * THOUSAND;
const auto ITERATIONS = MILLION;// 100 * THOUSAND;

/* Returns microseconds since epoch */
uint64_t timestamp_now()
{
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
}

struct Latencies
{
  Latencies()
  {
    minimum = std::numeric_limits<uint64_t>::max();
    maximum = std::numeric_limits<uint64_t>::min();
    sum = 0;
  }

  uint64_t minimum;
  uint64_t maximum;
  uint64_t sum;
};

template < typename Function >
Latencies runLogBenchmark(Function&& f, char const* msg)
{
  Latencies latencies;
  for (int i = 0; i < ITERATIONS; ++i)
  {
    auto begin = timestamp_now();
    f(i, msg);
    auto end = timestamp_now();
    auto latency = end - begin;
    latencies.minimum = std::min(latencies.minimum, latency);
    latencies.maximum = std::max(latencies.maximum, latency);
    latencies.sum += latency;

  }

  printf("Latency numbers in microseconds for %d iterations: \n"
         "%9s|%9s|%9s|\n"
         "%9lld|%9lld|%9lf|\n",
         ITERATIONS,
         "Min", "Max", "Average",
         latencies.minimum, latencies.maximum, (latencies.sum * 1.0) / ITERATIONS
         );
  return latencies;
}

void main()
{
  logger::registry().registerHandle(std::make_unique< logger::details::MultithreadRegistryHandle >());

  auto logger = std::make_shared< logger::Logger >("");
  logger->sink = logger::registry()->getSinkFactory()->createFileSink("benchmark.log", logger::StandardFormatter());
  //logger::registry()->registerLogger(logger);

  logger->filteringLevel = logger::Level::DEBUG;

  


  auto benchmark = [&logger](int i, const char* msg)
  {
    logger->debug(logger::LOGGER_CALL_CONTEXT, [&]()->std::string
    {
      // will be improve
      return "iteration #" + std::to_string(i) + std::string(", message: ") + std::string(msg);
    }
    );
  };

  runLogBenchmark(benchmark, "simple");

  logger::registry().unregisterHandle();
}
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
  implementation inspired by NanoLog benchmark
  (https://github.com/Iyengar111/NanoLog/blob/master/nano_vs_spdlog_vs_g3log_vs_reckless.cpp)
*/

const auto THOUSAND = 1000;
const auto MILLION = THOUSAND * THOUSAND;
//const auto ITERATIONS = 250 * THOUSAND;// MILLION;
const auto ITERATIONS = MILLION;

/* Returns microseconds since epoch */
uint64_t timestamp_now()
{
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
}

struct Latencies
{
  uint64_t minimum = std::numeric_limits<uint64_t>::max();
  uint64_t maximum = std::numeric_limits<uint64_t>::min();
  uint64_t sum = 0;
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
         "%9s|%9s|%9s|%13s|\n"
         "%9lld|%9lld|%9lf|%13lfs|\n",
         ITERATIONS,
         "Min", "Max", "Average", "Total (sec)",
         latencies.minimum, latencies.maximum, (latencies.sum * 1.0) / ITERATIONS, (latencies.sum * 0.000001)
         );
  return latencies;
}

template < typename Function >
void runBenchmark(Function&& f, int thread_count, char const * msg)
{
  printf("\nThread count: %d\n", thread_count);
  std::vector < std::thread > threads;
  for (int i = 0; i < thread_count; ++i)
  {
    threads.emplace_back(runLogBenchmark<Function>, std::ref(f), msg);
  }
  for (int i = 0; i < thread_count; ++i)
  {
    threads[i].join();
  }
}

void main()
{
  logger::registry().registerHandle(std::make_unique< logger::details::MultithreadRegistryHandle >());

  
  auto fileSink = logger::registry()->getSinkFactory()->createFileSink("benchmark.log", logger::StandardFormatter());

  for (auto threads : { 10 })
  {
    auto logger = std::make_shared< logger::Logger >("logger-" + std::to_string(threads));
    logger->sink = fileSink;
    logger->filteringLevel = logger::Level::DEBUG;

    auto benchmark = [&logger](int i, const char* msg)
    {
      logger->debug(logger::LOGGER_CALL_CONTEXT, [&]()->std::string
      {
        // will be improved
        return "iteration #" + std::to_string(i);// +std::string(", message: ") + std::string(msg);
      }
      );
    };


    runBenchmark(benchmark, threads, "simple");
  }

  //printf("waiting for to write all messages to out file...\n");
  //logger->flush();
  //printf("done\n");

  logger::registry().unregisterHandle();
}
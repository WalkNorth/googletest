// Copyright 2026 Google Inc. All rights reserved.
//
// SPSC Queue Performance Benchmark
//
// This example demonstrates performance characteristics of the SPSC queue
// through benchmarking with various message counts and queue sizes.

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <algorithm>

#include "spsc_queue.h"

// Benchmark configuration
struct BenchmarkConfig {
  const char* name;
  int message_count;
  size_t queue_capacity;
  int iterations;
};

// Run a single benchmark
struct BenchmarkResult {
  const char* name;
  int message_count;
  size_t queue_capacity;
  long long total_ns;
  long long avg_ns;
  long long min_ns;
  long long max_ns;
  double throughput_msg_per_sec;
};

BenchmarkResult run_benchmark(const BenchmarkConfig& config) {
  std::vector<long long> times;
  times.reserve(config.iterations);

  for (int iter = 0; iter < config.iterations; ++iter) {
    SpscQueue<int, 64> queue;  // Fixed size, only varying message count
    std::vector<int> received;
    received.reserve(config.message_count);

    auto start = std::chrono::high_resolution_clock::now();

    // Consumer thread
    std::thread consumer([&] {
      int value;
      while (received.size() < static_cast<size_t>(config.message_count)) {
        if (queue.pop(&value)) {
          received.push_back(value);
        } else {
          std::this_thread::yield();
        }
      }
    });

    // Producer thread
    std::thread producer([&] {
      for (int i = 0; i < config.message_count; ++i) {
        while (!queue.push(i)) {
          std::this_thread::yield();
        }
      }
    });

    producer.join();
    consumer.join();

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    long long elapsed_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    times.push_back(elapsed_ns);
  }

  // Calculate statistics
  long long total = std::accumulate(times.begin(), times.end(), 0LL);
  long long avg = total / config.iterations;
  long long min = *std::min_element(times.begin(), times.end());
  long long max = *std::max_element(times.begin(), times.end());
  double throughput =
      (config.message_count * config.iterations * 1e9) / total;

  return {config.name, config.message_count, config.queue_capacity,
          total, avg, min, max, throughput};
}

void print_result(const BenchmarkResult& result) {
  std::cout << std::left << std::setw(30) << result.name
            << std::setw(12) << result.message_count
            << std::setw(15) << result.queue_capacity
            << std::setw(15) << std::fixed << std::setprecision(0)
            << (result.avg_ns / 1000.0) << " us"
            << std::setw(12) << std::fixed << std::setprecision(2)
            << (result.throughput_msg_per_sec / 1e6) << " M/sec"
            << std::endl;
}

int main() {
  std::cout << "=== SPSC Queue Performance Benchmark ===" << std::endl;
  std::cout << std::endl;

  // Warmup
  {
    std::cout << "Running warmup iterations..." << std::endl;
    BenchmarkConfig warmup = {"warmup", 1000, 64, 3};
    run_benchmark(warmup);
    std::cout << "Warmup complete." << std::endl << std::endl;
  }

  // Benchmark suite
  std::vector<BenchmarkConfig> benchmarks = {
      {"Small (100 msg)", 100, 64, 10},
      {"Medium (1k msg)", 1000, 64, 10},
      {"Large (10k msg)", 10000, 64, 5},
      {"Very Large (100k msg)", 100000, 64, 3},
  };

  std::cout << std::left << std::setw(30) << "Test Name" << std::setw(12)
            << "Messages" << std::setw(15) << "Queue Cap"
            << std::setw(15) << "Avg Latency" << std::setw(12)
            << "Throughput" << std::endl;
  std::cout << std::string(84, '-') << std::endl;

  std::vector<BenchmarkResult> results;
  for (const auto& config : benchmarks) {
    auto result = run_benchmark(config);
    results.push_back(result);
    print_result(result);
  }

  std::cout << std::endl;
  std::cout << "=== Benchmark Summary ===" << std::endl;
  std::cout << "Peak throughput: " << std::fixed << std::setprecision(2)
            << (*std::max_element(results.begin(), results.end(),
                                   [](const auto& a, const auto& b) {
                                     return a.throughput_msg_per_sec <
                                            b.throughput_msg_per_sec;
                                   }))
                   .throughput_msg_per_sec / 1e6
            << " M messages/sec" << std::endl;

  return 0;
}

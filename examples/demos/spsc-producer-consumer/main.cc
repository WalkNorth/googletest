// Copyright 2026 Google Inc. All rights reserved.
//
// Simple SPSC Queue Producer-Consumer Example
//
// This example demonstrates basic usage of the SPSC queue with a producer
// and consumer thread.

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <iomanip>

#include "spsc_queue.h"

// Delay helper for demonstrations
void busy_wait_ns(long nanoseconds) {
  auto start = std::chrono::high_resolution_clock::now();
  while ((std::chrono::high_resolution_clock::now() - start).count() <
         nanoseconds) {
    // Busy wait
  }
}

int main() {
  std::cout << "=== SPSC Queue Producer-Consumer Demo ===" << std::endl;
  std::cout << std::endl;

  // Example 1: Basic push and pop
  {
    std::cout << "Example 1: Basic Push and Pop (Single Thread)" << std::endl;
    SpscQueue<int, 8> queue;

    // Push some values
    std::cout << "  Pushing: 10, 20, 30" << std::endl;
    queue.push(10);
    queue.push(20);
    queue.push(30);
    std::cout << "  Queue size: " << queue.size() << std::endl;

    // Pop values
    int value = 0;
    std::cout << "  Popping: ";
    while (queue.pop(&value)) {
      std::cout << value << " ";
    }
    std::cout << std::endl;
    std::cout << "  Queue empty: " << (queue.empty() ? "yes" : "no")
              << std::endl;
    std::cout << std::endl;
  }

  // Example 2: Full queue behavior
  {
    std::cout << "Example 2: Queue Capacity and Full Behavior" << std::endl;
    SpscQueue<int, 4> queue;

    std::cout << "  Queue capacity: " << queue.capacity() << std::endl;
    std::cout << "  Filling queue..." << std::endl;

    for (int i = 0; i < 5; ++i) {
      bool success = queue.push(i);
      std::cout << "    Push(" << i << "): " << (success ? "OK" : "FULL")
                << ", size=" << queue.size() << std::endl;
    }
    std::cout << std::endl;
  }

  // Example 3: Producer-Consumer with threads
  {
    std::cout << "Example 3: Producer-Consumer with Threads" << std::endl;
    constexpr int kNumMessages = 100;
    SpscQueue<int, 64> queue;
    std::vector<int> received;
    received.reserve(kNumMessages);

    std::cout << "  Spawning producer and consumer threads..." << std::endl;
    std::cout << "  Sending " << kNumMessages << " messages" << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    // Consumer thread
    std::thread consumer([&] {
      int value;
      while (received.size() < kNumMessages) {
        if (queue.pop(&value)) {
          received.push_back(value);
        } else {
          std::this_thread::yield();
        }
      }
    });

    // Producer thread
    std::thread producer([&] {
      for (int i = 0; i < kNumMessages; ++i) {
        while (!queue.push(i)) {
          std::this_thread::yield();
        }
        // Simulate some work
        if (i % 10 == 0) {
          busy_wait_ns(100);
        }
      }
    });

    producer.join();
    consumer.join();

    auto elapsed =
        std::chrono::high_resolution_clock::now() - start_time;
    auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

    std::cout << "  Completed in " << elapsed_ms.count() << " ms"
              << std::endl;
    std::cout << "  Messages received: " << received.size() << std::endl;

    // Verify received messages
    bool all_correct = true;
    for (int i = 0; i < kNumMessages; ++i) {
      if (received[i] != i) {
        all_correct = false;
        break;
      }
    }
    std::cout << "  Message order verification: "
              << (all_correct ? "PASS" : "FAIL") << std::endl;
    std::cout << std::endl;
  }

  // Example 4: Different data types
  {
    std::cout << "Example 4: Using Different Data Types" << std::endl;

    struct Message {
      int id;
      double value;
      char type;
    };

    SpscQueue<Message, 8> queue;

    Message msg{42, 3.14, 'X'};
    if (queue.push(msg)) {
      Message received;
      if (queue.pop(&received)) {
        std::cout << "  Received: id=" << received.id
                  << ", value=" << received.value
                  << ", type=" << received.type << std::endl;
      }
    }
    std::cout << std::endl;
  }

  std::cout << "=== Demo Complete ===" << std::endl;
  return 0;
}

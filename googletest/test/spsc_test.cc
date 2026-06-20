// Copyright 2026
// Simple single-producer / single-consumer queue used by unit tests.

#include <array>
#include <atomic>
#include <cstddef>
#include <new>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

namespace {

#if defined(__cpp_lib_hardware_interference_size)
constexpr size_t kCacheLineSize = std::hardware_destructive_interference_size;
#else
constexpr size_t kCacheLineSize = 64;
#endif

template <typename T>
struct alignas(kCacheLineSize) PaddedAtomic {
  PaddedAtomic(T initial = T{}) : value(initial) {}
  std::atomic<T> value;
  char pad[kCacheLineSize > sizeof(std::atomic<T>) ?
            kCacheLineSize - sizeof(std::atomic<T>) :
            1];
};

template <typename T, size_t N>
class SpscQueue {
 public:
  static_assert(N >= 2 && ((N & (N - 1)) == 0),
                "SpscQueue capacity must be a power of two and at least 2");

  SpscQueue()
      : head_(0),
        tail_(0) {}

  bool push(const T& value) {
    const size_t tail = tail_.value.load(std::memory_order_relaxed);
    const size_t head = head_.value.load(std::memory_order_acquire);
    if (tail - head == capacity_) {
      return false;
    }
    buffer_[tail & mask_] = value;
    tail_.value.store(tail + 1, std::memory_order_release);
    return true;
  }

  bool pop(T* value) {
    const size_t head = head_.value.load(std::memory_order_relaxed);
    const size_t tail = tail_.value.load(std::memory_order_acquire);
    if (tail == head) {
      return false;
    }
    *value = buffer_[head & mask_];
    head_.value.store(head + 1, std::memory_order_release);
    return true;
  }

  bool empty() const {
    return head_.value.load(std::memory_order_acquire) ==
           tail_.value.load(std::memory_order_acquire);
  }

  bool full() const {
    return (tail_.value.load(std::memory_order_relaxed) -
            head_.value.load(std::memory_order_acquire)) == capacity_;
  }

 private:
  static constexpr size_t capacity_ = N;
  static constexpr size_t mask_ = capacity_ - 1;
  // Use monotonically increasing head/tail counters and mask them when
  // indexing the buffer. This lets `N` be the usable capacity (must be a
  // power of two) while avoiding the extra reserved slot.

  std::array<T, capacity_> buffer_;
  PaddedAtomic<size_t> head_;
  PaddedAtomic<size_t> tail_;
};

TEST(SpscQueueTest, PushPopSingleThread) {
  SpscQueue<int, 4> queue;

  EXPECT_TRUE(queue.empty());
  EXPECT_FALSE(queue.full());

  EXPECT_TRUE(queue.push(42));
  EXPECT_FALSE(queue.empty());

  int value = 0;
  EXPECT_TRUE(queue.pop(&value));
  EXPECT_EQ(value, 42);
  EXPECT_TRUE(queue.empty());
}

TEST(SpscQueueTest, FullEmptyBehavior) {
  SpscQueue<int, 2> queue;

  EXPECT_TRUE(queue.push(1));
  EXPECT_TRUE(queue.push(2));
  EXPECT_TRUE(queue.full());
  EXPECT_FALSE(queue.push(3));

  int value = 0;
  EXPECT_TRUE(queue.pop(&value));
  EXPECT_EQ(value, 1);
  EXPECT_FALSE(queue.full());
  EXPECT_TRUE(queue.push(3));
  EXPECT_TRUE(queue.full());

  EXPECT_TRUE(queue.pop(&value));
  EXPECT_EQ(value, 2);
  EXPECT_TRUE(queue.pop(&value));
  EXPECT_EQ(value, 3);
  EXPECT_TRUE(queue.empty());
}

TEST(SpscQueueTest, SingleProducerSingleConsumerThreaded) {
  constexpr int kMessages = 1000;
  SpscQueue<int, 64> queue;
  std::vector<int> consumed;
  consumed.reserve(kMessages);
  std::atomic<bool> producer_done(false);

  std::thread consumer([&] {
    while (!producer_done.load(std::memory_order_acquire) || !queue.empty()) {
      int value;
      if (queue.pop(&value)) {
        consumed.push_back(value);
      } else {
        std::this_thread::yield();
      }
    }
  });

  std::thread producer([&] {
    for (int i = 0; i < kMessages; ++i) {
      while (!queue.push(i)) {
        std::this_thread::yield();
      }
    }
    producer_done.store(true, std::memory_order_release);
  });

  producer.join();
  consumer.join();

  EXPECT_EQ(static_cast<int>(consumed.size()), kMessages);
  for (int i = 0; i < kMessages; ++i) {
    EXPECT_EQ(consumed[i], i);
  }
}

}  // namespace

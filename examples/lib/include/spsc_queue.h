// Copyright 2026 Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef EXAMPLES_LIB_INCLUDE_SPSC_QUEUE_H_
#define EXAMPLES_LIB_INCLUDE_SPSC_QUEUE_H_

#include <array>
#include <atomic>
#include <cstddef>

// Determine cache line size for proper alignment
#if defined(__cpp_lib_hardware_interference_size)
constexpr size_t kCacheLineSize = std::hardware_destructive_interference_size;
#else
constexpr size_t kCacheLineSize = 64;
#endif

// Atomic with cache line padding to prevent false sharing
template <typename T>
struct alignas(kCacheLineSize) PaddedAtomic {
  PaddedAtomic(T initial = T{}) : value(initial) {}
  std::atomic<T> value;
  char pad[kCacheLineSize > sizeof(std::atomic<T>)
               ? kCacheLineSize - sizeof(std::atomic<T>)
               : 1];
};

// Single-producer, single-consumer wait-free queue
// Template parameters:
//   T: Data type to store in the queue
//   N: Capacity of the queue (must be a power of 2 and >= 2)
//
// This is a lock-free SPSC queue suitable for high-performance scenarios
// where a single producer and single consumer communicate.
template <typename T, size_t N>
class SpscQueue {
 public:
  // Verify that capacity is a power of two
  static_assert(N >= 2 && ((N & (N - 1)) == 0),
                "SpscQueue capacity must be a power of two and at least 2");

  SpscQueue() : head_(0), tail_(0) {}

  // Non-copyable and non-movable
  SpscQueue(const SpscQueue&) = delete;
  SpscQueue& operator=(const SpscQueue&) = delete;

  // Atomically push a value to the queue (producer side)
  // Returns true if successful, false if queue is full
  bool push(const T& value) {
    const size_t tail = tail_.value.load(std::memory_order_relaxed);
    const size_t head = head_.value.load(std::memory_order_acquire);
    if (tail - head == capacity_) {
      return false;  // Queue is full
    }
    buffer_[tail & mask_] = value;
    tail_.value.store(tail + 1, std::memory_order_release);
    return true;
  }

  // Atomically pop a value from the queue (consumer side)
  // Returns true if successful, false if queue is empty
  bool pop(T* value) {
    const size_t head = head_.value.load(std::memory_order_relaxed);
    const size_t tail = tail_.value.load(std::memory_order_acquire);
    if (tail == head) {
      return false;  // Queue is empty
    }
    *value = buffer_[head & mask_];
    head_.value.store(head + 1, std::memory_order_release);
    return true;
  }

  // Check if queue is empty (consumer side)
  bool empty() const {
    return head_.value.load(std::memory_order_acquire) ==
           tail_.value.load(std::memory_order_acquire);
  }

  // Check if queue is full (producer side)
  bool full() const {
    return (tail_.value.load(std::memory_order_relaxed) -
            head_.value.load(std::memory_order_acquire)) == capacity_;
  }

  // Get current queue size (approximate, lock-free)
  size_t size() const {
    return tail_.value.load(std::memory_order_acquire) -
           head_.value.load(std::memory_order_acquire);
  }

  // Get queue capacity
  static constexpr size_t capacity() { return capacity_; }

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

#endif  // EXAMPLES_LIB_INCLUDE_SPSC_QUEUE_H_

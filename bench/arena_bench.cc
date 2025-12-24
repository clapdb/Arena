/*
 * Copyright (c) 2024 ClapDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include "nanobench.h"

#include "arena/arena.hpp"

#include <cstdlib>
#include <memory>
#include <memory_resource>
#include <random>
#include <vector>

using namespace arena;
namespace nb = ankerl::nanobench;

// Test object with destructor
struct TestObject {
  int data[4];
  ArenaFullManagedTag;
  TestObject() { data[0] = 42; }
  ~TestObject() { data[0] = 0; }
};

// Test object without destructor registration
struct SimpleObject {
  int data[4];
  ArenaManagedCreateOnlyTag;
  SimpleObject() { data[0] = 42; }
  ~SimpleObject() = default;
};

int main() {
  std::cout << "======================================================\n";
  std::cout << "           Arena vs Malloc Benchmark Suite\n";
  std::cout << "======================================================\n\n";

  // ========================================================================
  // Benchmark 1: Small allocations (32 bytes)
  // ========================================================================
  {
    nb::Bench bench;
    bench.title("Small Allocation (32B)")
        .warmup(1000)
        .minEpochIterations(100000);

    Arena arena(Arena::Options::GetDefaultOptions());
    bench.run("Arena::AllocateAligned", [&]() {
      void *p = arena.AllocateAligned(32);
      nb::doNotOptimizeAway(p);
    });

    std::vector<void *> ptrs;
    ptrs.reserve(1000000);
    bench.run("malloc", [&]() {
      void *p = std::malloc(32);
      nb::doNotOptimizeAway(p);
      ptrs.push_back(p);
    });
    for (auto *p : ptrs)
      std::free(p);
  }

  // ========================================================================
  // Benchmark 2: Medium allocations (512 bytes)
  // ========================================================================
  {
    nb::Bench bench;
    bench.title("Medium Allocation (512B)")
        .warmup(1000)
        .minEpochIterations(50000);

    Arena arena(Arena::Options::GetDefaultOptions());
    bench.run("Arena::AllocateAligned", [&]() {
      void *p = arena.AllocateAligned(512);
      nb::doNotOptimizeAway(p);
    });

    std::vector<void *> ptrs;
    ptrs.reserve(500000);
    bench.run("malloc", [&]() {
      void *p = std::malloc(512);
      nb::doNotOptimizeAway(p);
      ptrs.push_back(p);
    });
    for (auto *p : ptrs)
      std::free(p);
  }

  // ========================================================================
  // Benchmark 3: Large allocations (4KB)
  // ========================================================================
  {
    nb::Bench bench;
    bench.title("Large Allocation (4KB)").warmup(100).minEpochIterations(10000);

    Arena::Options opts = Arena::Options::GetDefaultOptions();
    opts.huge_block_size = 64 * 1024 * 1024;
    Arena arena(opts);
    bench.run("Arena::AllocateAligned", [&]() {
      void *p = arena.AllocateAligned(4096);
      nb::doNotOptimizeAway(p);
    });

    std::vector<void *> ptrs;
    ptrs.reserve(100000);
    bench.run("malloc", [&]() {
      void *p = std::malloc(4096);
      nb::doNotOptimizeAway(p);
      ptrs.push_back(p);
    });
    for (auto *p : ptrs)
      std::free(p);
  }

  // ========================================================================
  // Benchmark 4: Batch allocation + reset/free (key Arena use case)
  // ========================================================================
  {
    nb::Bench bench;
    bench.title("Batch Alloc+Free (64B x 1000)")
        .warmup(100)
        .minEpochIterations(1000);

    Arena arena(Arena::Options::GetDefaultOptions());
    bench.run("Arena + Reset", [&]() {
      for (int i = 0; i < 1000; ++i) {
        void *p = arena.AllocateAligned(64);
        nb::doNotOptimizeAway(p);
      }
      arena.Reset();
    });

    bench.run("malloc + free", [&]() {
      std::vector<void *> ptrs;
      ptrs.reserve(1000);
      for (int i = 0; i < 1000; ++i) {
        void *p = std::malloc(64);
        nb::doNotOptimizeAway(p);
        ptrs.push_back(p);
      }
      for (auto *p : ptrs)
        std::free(p);
    });
  }

  // ========================================================================
  // Benchmark 5: Object creation with destructor
  // ========================================================================
  {
    nb::Bench bench;
    bench.title("Object Create<T> with destructor")
        .warmup(1000)
        .minEpochIterations(50000);

    Arena arena(Arena::Options::GetDefaultOptions());
    bench.run("Arena::Create<T>", [&]() {
      auto *p = arena.Create<TestObject>();
      nb::doNotOptimizeAway(p);
    });

    std::vector<TestObject *> ptrs;
    ptrs.reserve(500000);
    bench.run("new T", [&]() {
      auto *p = new TestObject();
      nb::doNotOptimizeAway(p);
      ptrs.push_back(p);
    });
    for (auto *p : ptrs)
      delete p;
  }

  // ========================================================================
  // Benchmark 6: Object creation without destructor
  // ========================================================================
  {
    nb::Bench bench;
    bench.title("Object Create<T> skip destructor")
        .warmup(1000)
        .minEpochIterations(50000);

    Arena arena(Arena::Options::GetDefaultOptions());
    bench.run("Arena::Create<T>", [&]() {
      auto *p = arena.Create<SimpleObject>();
      nb::doNotOptimizeAway(p);
    });

    std::vector<SimpleObject *> ptrs;
    ptrs.reserve(500000);
    bench.run("new T", [&]() {
      auto *p = new SimpleObject();
      nb::doNotOptimizeAway(p);
      ptrs.push_back(p);
    });
    for (auto *p : ptrs)
      delete p;
  }

  // ========================================================================
  // Benchmark 7: PMR vector operations
  // ========================================================================
  {
    nb::Bench bench;
    bench.title("Vector push_back x100").warmup(100).minEpochIterations(10000);

    Arena arena(Arena::Options::GetDefaultOptions());
    bench.run("pmr::vector + Arena", [&]() {
      std::pmr::vector<int> vec(arena.get_memory_resource());
      for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
      }
      nb::doNotOptimizeAway(vec.data());
    });

    bench.run("std::vector", [&]() {
      std::vector<int> vec;
      for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
      }
      nb::doNotOptimizeAway(vec.data());
    });
  }

  // ========================================================================
  // Benchmark 8: Mixed size allocations
  // ========================================================================
  {
    nb::Bench bench;
    bench.title("Mixed Sizes (8-1024B)").warmup(1000).minEpochIterations(50000);

    std::vector<size_t> sizes = {8, 16, 32, 64, 128, 256, 512, 1024};
    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> dist(0, sizes.size() - 1);

    std::vector<size_t> random_sizes;
    random_sizes.reserve(100000);
    for (int i = 0; i < 100000; ++i) {
      random_sizes.push_back(sizes[dist(rng)]);
    }

    Arena arena(Arena::Options::GetDefaultOptions());
    size_t idx = 0;
    bench.run("Arena::AllocateAligned", [&]() {
      void *p =
          arena.AllocateAligned(random_sizes[idx++ % random_sizes.size()]);
      nb::doNotOptimizeAway(p);
    });

    std::vector<void *> ptrs;
    ptrs.reserve(500000);
    idx = 0;
    bench.run("malloc", [&]() {
      void *p = std::malloc(random_sizes[idx++ % random_sizes.size()]);
      nb::doNotOptimizeAway(p);
      ptrs.push_back(p);
    });
    for (auto *p : ptrs)
      std::free(p);
  }

  // ========================================================================
  // Benchmark 9: Allocation with memory touch
  // ========================================================================
  {
    nb::Bench bench;
    bench.title("Alloc + memset (64B)").warmup(1000).minEpochIterations(50000);

    Arena arena(Arena::Options::GetDefaultOptions());
    bench.run("Arena + touch", [&]() {
      char *p = arena.AllocateAligned(64);
      for (int i = 0; i < 64; i += 8) {
        p[i] = static_cast<char>(i);
      }
      nb::doNotOptimizeAway(p);
    });

    std::vector<void *> ptrs;
    ptrs.reserve(500000);
    bench.run("malloc + touch", [&]() {
      char *p = static_cast<char *>(std::malloc(64));
      for (int i = 0; i < 64; i += 8) {
        p[i] = static_cast<char>(i);
      }
      nb::doNotOptimizeAway(p);
      ptrs.push_back(p);
    });
    for (auto *p : ptrs)
      std::free(p);
  }

  // ========================================================================
  // Benchmark 10: Sequential pattern (simulates parsing)
  // ========================================================================
  {
    nb::Bench bench;
    bench.title("Parse Pattern (100 mixed allocs + reset/free)")
        .warmup(100)
        .minEpochIterations(1000);

    std::vector<size_t> pattern = {16, 32, 8, 64, 16, 128, 32, 16, 8, 256};

    Arena arena(Arena::Options::GetDefaultOptions());
    bench.run("Arena + Reset", [&]() {
      for (int i = 0; i < 100; ++i) {
        void *p = arena.AllocateAligned(pattern[i % pattern.size()]);
        nb::doNotOptimizeAway(p);
      }
      arena.Reset();
    });

    bench.run("malloc + free", [&]() {
      std::vector<void *> ptrs;
      ptrs.reserve(100);
      for (int i = 0; i < 100; ++i) {
        void *p = std::malloc(pattern[i % pattern.size()]);
        nb::doNotOptimizeAway(p);
        ptrs.push_back(p);
      }
      for (auto *p : ptrs)
        std::free(p);
    });
  }

  std::cout << "\n======================================================\n";
  std::cout << "                    Benchmark Complete\n";
  std::cout << "======================================================\n";
  std::cout << "\nNotes:\n";
  std::cout << "  - Lower time is better\n";
  std::cout << "  - Arena excels at batch allocations with Reset()\n";
  std::cout << "  - Results vary based on system allocator\n";
  std::cout << "  - Run with: ./arena_bench for detailed results\n\n";

  return 0;
}

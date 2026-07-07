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
#include "arena/metrics.hpp"

#include <atomic>
#include <cstdint>

#if defined(STDB_USE_FMT_MODULE)
#ifndef ARENA_FMT_IMPORTED
#define ARENA_FMT_IMPORTED 1
import fmt;
#endif
#else
#include <fmt/format.h>
#endif

namespace arena {

GlobalArenaMetrics global_arena_metrics = GlobalArenaMetrics();

thread_local LocalArenaMetrics local_arena_metrics = LocalArenaMetrics();

auto format_create_array_overflow(uint64_t num, const char *type_name, uint64_t type_size) -> std::string {
    return fmt::format("CreateArray need too many memory, that more than max of "
                       "uint64_t, the num of array is {}, and the Type "
                       "is {}, sizeof T is {}",
                       num, type_name, type_size);
}

auto GlobalArenaMetrics::string() const -> std::string {
    std::string str;
    str += fmt::format(
      "Summary:\n"
      "  init_count: {}\n"
      "  reset_count: {}\n"
      "  destruct_count: {}\n"
      "  alloc_count: {}\n"
      "  newblock_count: {}\n"
      "  space_allocated: {}\n"
      "  space_used: {}\n"
      "  space_wasted: {}\n"
      "  space_resettled: {}\nAllocSize distribution:",
      init_count.load(std::memory_order::relaxed), reset_count.load(std::memory_order::relaxed),
      destruct_count.load(std::memory_order::relaxed), alloc_count.load(std::memory_order::relaxed),
      newblock_count.load(std::memory_order::relaxed), space_allocated.load(std::memory_order::relaxed),
      space_used.load(std::memory_order::relaxed), space_wasted.load(std::memory_order::relaxed),
      space_resettled.load(std::memory_order::relaxed));

    constexpr uint64_t kPercentMagic = 100UL;
    const auto total_alloc_count = alloc_count.load(std::memory_order::relaxed);
    for (uint64_t i = 0, count = 0; i < kAllocBucketSize; i++) {
        count += alloc_size_bucket_counter.at(i).load(std::memory_order::relaxed);
        const auto percent = total_alloc_count == 0 ? 0 : count * kPercentMagic / total_alloc_count;
        str += fmt::format("\n  le={}: {}%", alloc_size_bucket.at(i), percent);
    }

    str += "\nLifetime distribution:";
    const auto total_destruct_count = destruct_count.load(std::memory_order::relaxed);
    for (uint64_t i = 0, count = 0; i < kLifetimeBucketSize; i++) {
        count += destruct_lifetime_bucket_counter.at(i).load(std::memory_order::relaxed);
        const auto percent = total_destruct_count == 0 ? 0 : count * kPercentMagic / total_destruct_count;
        str += fmt::format("\n  le={}ms: {}", destruct_lifetime_bucket.at(i).count(), percent);
    }

    str += "\nArena Location/AllocSize:";
    for (const auto& [loc, counter] : arena_alloc_counter) {
        str += fmt::format("\n  {}: {}", loc, counter.load(std::memory_order::relaxed));
    }

    return str;
}

}  // namespace arena

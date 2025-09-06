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
#include <fmt/format.h>

namespace arena {

GlobalArenaMetrics global_arena_metrics = GlobalArenaMetrics();

thread_local LocalArenaMetrics local_arena_metrics = LocalArenaMetrics();

}  // namespace arena

namespace fmt {
auto formatter<std::atomic<uint64_t>>::format(const std::atomic<uint64_t>& data, format_context& ctx) const noexcept
  -> decltype(ctx.out()) {
    uint64_t value = data.load(std::memory_order::relaxed);
    return formatter<uint64_t>::format(value, ctx);
}
}  // namespace fmt
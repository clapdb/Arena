module;
#include "arena/align.hpp"
#include "arena/arena.hpp"
#include "arena/memory.hpp"
#include "arena/metrics.hpp"

export module arena;

export namespace arena {
using ::arena::Arena;
using ::arena::ArenaContainStatus;
using ::arena::ArenaError;
using ::arena::ArenaMetricsCookie;
using ::arena::Expected;
using ::arena::GlobalArenaMetrics;
using ::arena::LocalArenaMetrics;
using ::arena::default_logger_func;
using ::arena::format_create_array_overflow;
using ::arena::global_arena_metrics;
using ::arena::local_arena_metrics;
using ::arena::metrics_probe_on_arena_allocation;
using ::arena::metrics_probe_on_arena_destruction;
using ::arena::metrics_probe_on_arena_init;
using ::arena::metrics_probe_on_arena_newblock;
using ::arena::metrics_probe_on_arena_reset;

namespace align {
using ::arena::align::AlignUp;
using ::arena::align::AlignUpTo;
using ::arena::align::kMinAlignSize;
}  // namespace align
}

export namespace stdb::memory::literals {
using ::stdb::memory::literals::operator""_GB;
using ::stdb::memory::literals::operator""_KB;
using ::stdb::memory::literals::operator""_MB;
}

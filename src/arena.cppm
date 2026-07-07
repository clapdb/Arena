module;
#include "arena/arena.hpp"
#include "arena/metrics.hpp"

export module arena;

export namespace arena {
using ::arena::Arena;
using ::arena::ArenaContainStatus;
using ::arena::ArenaError;
using ::arena::Expected;
using ::arena::GlobalArenaMetrics;
using ::arena::LocalArenaMetrics;
using ::arena::format_create_array_overflow;
using ::arena::global_arena_metrics;
using ::arena::local_arena_metrics;
}

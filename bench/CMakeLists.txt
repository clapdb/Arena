cmake_minimum_required(VERSION 3.10)

add_executable(arena_bench EXCLUDE_FROM_ALL arena_bench.cc)

target_include_directories(arena_bench PRIVATE
    ${CMAKE_SOURCE_DIR}/nanobench/src/include
)

target_compile_options(arena_bench PRIVATE
    -O3
    -DNDEBUG
    -frtti
)

# -march=native only when we are the project being benchmarked. Vendored into someone else's build it is
# not ours to choose: the parent picks the ISA level for its whole tree (often deliberately conservative,
# so the binary runs on machines other than the one that built it), and a benchmark quietly opting out of
# that is both a portability hazard and a misleading measurement -- it would no longer be measuring the
# code the parent actually ships.
if(PROJECT_IS_TOP_LEVEL)
    target_compile_options(arena_bench PRIVATE -march=native)
endif()

target_link_libraries(arena_bench arena)

cmake_minimum_required(VERSION 3.10)

add_executable(arena_bench EXCLUDE_FROM_ALL arena_bench.cc)

target_include_directories(arena_bench PRIVATE
    ${CMAKE_SOURCE_DIR}/nanobench/src/include
)

target_compile_options(arena_bench PRIVATE
    -O3
    -DNDEBUG
    -march=native
    -frtti
)

target_link_libraries(arena_bench arena)

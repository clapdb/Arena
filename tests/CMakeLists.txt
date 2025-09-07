#[[
 Copyright (C) 2020 hurricane <l@stdb.io>. All rights reserved.
  +------------------------------------------------------------------+
   |                                                                  |
   |                                                                  |
   |  #####                                                           |
   | #     # #####   ##   #####  #####  # #    # ######  ####   ####  |
   | #         #    #  #  #    # #    # # ##   # #      #      #      |
   |  #####    #   #    # #    # #    # # # #  # #####   ####   ####  |
   |       #   #   ###### #####  #####  # #  # # #           #      # |
   | #     #   #   #    # #   #  #   #  # #   ## #      #    # #    # |
   |  #####    #   #    # #    # #    # # #    # ######  ####   ####  |
   |                                                                  |
   |                                                                  |
   +------------------------------------------------------------------+
 ]]

cmake_minimum_required(VERSION 3.10)

aux_source_directory(. TEST_SRC)

add_executable(arena_test EXCLUDE_FROM_ALL ${TEST_SRC})


## test flags
list(APPEND TEST_FLAGS
  -DDOCTEST_CONFIG_SUPER_FAST_ASSERTS
  -Wno-deprecated
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-error=cpp")
### include arena src
#target_include_directories(memory_test PRIVATE ${MEMORY_SOURCE_DIR}/)

target_compile_options(arena_test PRIVATE ${TEST_FLAGS} -frtti)
target_link_libraries(arena_test arena dl)

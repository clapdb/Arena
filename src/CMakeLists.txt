#[[
 Copyright (C) 2020 hurricane <l@stdb.io>. All rights reserved.
 +------------------------------------------------------------------------------+
 |                                                                              |
 |                                                                              |
 |                    ..######..########.########..########.                    |
 |                    .##....##....##....##.....##.##.....##                    |
 |                    .##..........##....##.....##.##.....##                    |
 |                    ..######.....##....##.....##.########.                    |
 |                    .......##....##....##.....##.##.....##                    |
 |                    .##....##....##....##.....##.##.....##                    |
 |                    ..######.....##....########..########.                    |
 |                                                                              |
 |                                                                              |
 |                                                                              |
 +------------------------------------------------------------------------------+
]]

aux_source_directory(. ARENA_SRC)
list(FILTER ARENA_SRC EXCLUDE REGEX "arena\\.cppm$")
add_library(arena ${ARENA_SRC})
target_compile_options(arena PRIVATE -frtti)
target_link_libraries(arena PRIVATE fmt::fmt)
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.28 AND CMAKE_CXX_SCAN_FOR_MODULES)
    target_sources(arena
        PUBLIC
            FILE_SET CXX_MODULES
            FILES arena.cppm
    )
endif()

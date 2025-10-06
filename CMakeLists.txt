cmake_minimum_required(VERSION 3.10)

project(Arena
    VERSION 1.0.0
    DESCRIPTION "A C++20 memory allocation library inspired by the Chromium project"
    LANGUAGES CXX
)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Global compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-error=cpp -frtti")

# Add include directories
include_directories(include)
include_directories(doctest)
include_directories(fmt/include)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)
if(NOT TARGET fmt::fmt)
    add_subdirectory(fmt)
endif()

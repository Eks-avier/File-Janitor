cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(FileJanitor LANGUAGES CXX)

add_library(Headers INTERFACE)
target_include_directories(Headers INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Headers INTERFACE stdc++exp)

# Set compiler warnings here
target_compile_options(
  Headers
  INTERFACE -fno-exceptions
            # Core warnings
            -Wall
            -Wextra
            -Wpedantic
            -pedantic-errors
            -Werror
            # Modern C++ best practices
            -Weffc++
            -Wold-style-cast
            -Woverloaded-virtual
            -Wuseless-cast
            # Type safety
            -Wconversion
            -Wsign-conversion
            -Wcast-align
            # Additional safety checks
            -Wshadow
            -Wnon-virtual-dtor
            -Wnull-dereference
            -Wformat=2
            -Wundef
            -Wduplicated-cond
            -Wduplicated-branches
            -Wmisleading-indentation)

# Add GCC-specific warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(Headers INTERFACE -Wlogical-op)
endif()

# Enable libstdc++ assertions in Debug builds
target_compile_definitions(
  Headers INTERFACE $<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>)

# Find third-party dependencies
find_package(fmt CONFIG REQUIRED)
find_package(scn CONFIG REQUIRED)

add_executable(Main)
target_sources(Main PRIVATE src/main.cxx)
target_link_libraries(Main PRIVATE Headers fmt::fmt scn::scn)

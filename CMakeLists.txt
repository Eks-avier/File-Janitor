cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(FileJanitor LANGUAGES CXX)

add_library(Headers INTERFACE
        include/fs_ops.hxx
        include/safe_fs.hxx
        include/result_types.hxx)
target_include_directories(Headers INTERFACE ${CMAKE_SOURCE_DIR}/include)

target_compile_features(Headers INTERFACE cxx_std_23)

# Common warnings supported by both GCC and Clang
target_compile_options(
  Headers
  INTERFACE
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
            -Wmisleading-indentation)

# GCC-specific warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(
    Headers
    INTERFACE -Wlogical-op # Warn about suspicious uses of logical operators
              -Wuseless-cast # Warn about useless casts
              -Wduplicated-cond # Warn about duplicated conditions in if-else
              -Wduplicated-branches) # Warn about duplicated branches
#              $<$<CONFIG:Debug>:-fanalyzer>) # Static analysis in Debug builds
endif()

# Clang-specific warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
    Headers
    INTERFACE -Wthread-safety # Clang's excellent thread safety analysis
              -Wloop-analysis # Detect suspicious loop constructs
              -Wno-missing-braces  # Clang is overly strict on aggregate init
             )
    target_link_libraries(Headers INTERFACE stdc++exp)
endif()

# Enable libstdc++ assertions in Debug builds
target_compile_definitions(
  Headers INTERFACE $<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>)

# Find third-party dependencies
find_package(fmt CONFIG REQUIRED)
find_package(ctre CONFIG REQUIRED)
find_package(scn CONFIG REQUIRED)

add_executable(Main
        src/fs_ops.cxx
        src/safe_fs.cxx)
target_sources(Main PRIVATE
  src/main.cxx
)

target_link_libraries(Main PRIVATE Headers fmt::fmt)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(Main PRIVATE stdc++exp)
endif()

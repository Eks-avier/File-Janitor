cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD d0edc3af-4c50-42ea-a356-e2862fe7a444)
set(CMAKE_CXX_MODULE_STD ON)

project(FileJanitor LANGUAGES CXX)

add_library(Headers INTERFACE)
target_include_directories(Headers INTERFACE ${CMAKE_SOURCE_DIR}/include)

target_compile_features(Headers INTERFACE cxx_std_26)

# Common warnings supported by both GCC and Clang
target_compile_options(
        Headers
        INTERFACE
        -fmodules
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
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(
          Headers
          INTERFACE -Wlogical-op # Warn about suspicious uses of logical operators
          -Wuseless-cast # Warn about useless casts
          -Wduplicated-cond # Warn about duplicated conditions in if-else
          -Wduplicated-branches) # Warn about duplicated branches
  #              $<$<CONFIG:Debug>:-fanalyzer>) # Static analysis in Debug builds
endif ()

# Clang-specific warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
          Headers
          INTERFACE -Wthread-safety # Clang's excellent thread safety analysis
          -Wloop-analysis # Detect suspicious loop constructs
          -Wno-missing-braces  # Clang is overly strict on aggregate init
  )
  target_link_libraries(Headers INTERFACE stdc++exp)
endif ()

# Enable libstdc++ assertions in Debug builds
target_compile_definitions(
        Headers INTERFACE $<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>)

# Find third-party dependencies
find_package(fmt CONFIG REQUIRED)
find_package(scn CONFIG REQUIRED)

set(FS_OPS_PATH src/fs_ops)
set(FS_OPS_PLANNER_PATH ${FS_OPS_PATH}/planner)
set(FS_OPS_SCANNER_PATH ${FS_OPS_PATH}/scanner)
set(FS_OPS_EXECUTOR_PATH ${FS_OPS_PATH}/executor)

add_library(fs_ops_headers INTERFACE)
target_include_directories(fs_ops_headers INTERFACE ${CMAKE_SOURCE_DIR}/include/fs_ops)
target_compile_features(fs_ops_headers INTERFACE cxx_std_26)
target_compile_options(fs_ops_headers INTERFACE -fmodules)

add_library(scanner)
target_sources(scanner PRIVATE ${FS_OPS_SCANNER_PATH}/scanner.cxx)
target_link_libraries(scanner PRIVATE fs_ops_headers)

add_library(planner)
target_sources(planner PRIVATE ${FS_OPS_PLANNER_PATH}/planner.cxx)
target_link_libraries(planner PRIVATE fs_ops_headers)

add_library(executor)
target_sources(executor PRIVATE ${FS_OPS_EXECUTOR_PATH}/executor.cxx)
target_link_libraries(executor PRIVATE fs_ops_headers PRIVATE fmt::fmt PRIVATE Headers)

add_library(fs_ops)
target_sources(fs_ops PRIVATE src/fs_ops/operation_result.cpp)
target_link_libraries(fs_ops PUBLIC fs_ops_headers)

add_executable(Main src/main.cxx)
target_sources(Main PRIVATE
        src/safe_fs.cxx
)
target_link_libraries(Main PRIVATE Headers fmt::fmt fs_ops)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(Main PRIVATE stdc++exp)
endif ()

# TEST FOLDER STUFF
# 1. Define the source and destination
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/sandbox")
set(DEST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/sandbox")

# 2. Copy resources to the build folder during configuration
file(COPY ${RESOURCE_DIR} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
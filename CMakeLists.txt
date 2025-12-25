cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

# Enable C++20 modules with import std support
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD d0edc3af-4c50-42ea-a356-e2862fe7a444)
set(CMAKE_CXX_MODULE_STD ON)

project(FileJanitor LANGUAGES CXX)

# =============================================================================
# COMPILE SETTINGS: Common compilation flags and features
# =============================================================================
add_library(CompileSettings INTERFACE)
target_compile_features(CompileSettings INTERFACE cxx_std_26)

# Common warnings supported by both GCC and Clang
target_compile_options(
        CompileSettings
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
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(
            CompileSettings
            INTERFACE
            -Wlogical-op
            -Wuseless-cast
            -Wduplicated-cond
            -Wduplicated-branches)
endif ()

# Clang-specific warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(
            CompileSettings
            INTERFACE
            -Wthread-safety
            -Wloop-analysis
            -Wno-missing-braces)
endif ()

# Enable libstdc++ assertions in Debug builds
target_compile_definitions(
        CompileSettings INTERFACE $<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>)

# =============================================================================
# THIRD-PARTY DEPENDENCIES
# =============================================================================
find_package(fmt CONFIG REQUIRED)
find_package(scn CONFIG REQUIRED)

# =============================================================================
# MODULE LIBRARY: filejanitor
# =============================================================================
add_library(filejanitor)

# Module interface units (CXX_MODULES file set)
target_sources(filejanitor
        PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_SOURCE_DIR}/modules
        FILES
        # Primary module interface
        modules/filejanitor.cppm

        # Foundation partitions (no internal deps)
        modules/filejanitor-result_types.cppm
        modules/filejanitor-fs_ops.cppm
        modules/filejanitor-scanner.cppm

        # Dependency layer partitions
        modules/filejanitor-safe_fs.cppm
        modules/filejanitor-movement_plan.cppm
        modules/filejanitor-operation_result.cppm
        modules/filejanitor-execution_report.cppm

        # Aggregation layer partitions
        modules/filejanitor-planner.cppm
        modules/filejanitor-executor.cppm
)

# Implementation sources
target_sources(filejanitor
        PRIVATE
        src/safe_fs.cxx
        src/fs_ops/scanner/scanner.cxx
        src/fs_ops/planner/planner.cxx
        src/fs_ops/executor/executor.cxx
        src/fs_ops/executor/execution_report.cxx
        src/fs_ops/operation_result.cpp
)

# Link dependencies
target_link_libraries(filejanitor
        PUBLIC CompileSettings
        PRIVATE fmt::fmt)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(filejanitor PRIVATE stdc++exp)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_libraries(filejanitor PRIVATE stdc++exp)
endif ()

# =============================================================================
# EXECUTABLE: Main
# =============================================================================
add_executable(Main src/main.cxx)
target_link_libraries(Main
        PRIVATE filejanitor
        PRIVATE fmt::fmt)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(Main PRIVATE stdc++exp)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_libraries(Main PRIVATE stdc++exp)
endif ()

# =============================================================================
# TEST RESOURCES
# =============================================================================
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/sandbox")
set(DEST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/sandbox")
file(COPY ${RESOURCE_DIR} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

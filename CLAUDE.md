# CLAUDE.md - AI Assistant Guide for File-Janitor

This document provides comprehensive guidance for AI assistants working on the File-Janitor codebase.

## Table of Contents

1. [Project Overview](#project-overview)
2. [Codebase Structure](#codebase-structure)
3. [Architecture & Design Patterns](#architecture--design-patterns)
4. [Development Workflows](#development-workflows)
5. [Code Conventions](#code-conventions)
6. [Build System](#build-system)
7. [Testing Guidelines](#testing-guidelines)
8. [Common Tasks](#common-tasks)
9. [Troubleshooting](#troubleshooting)

---

## Project Overview

**File-Janitor** is a modern C++ command-line utility that organizes files by extension into categorized directories ("buckets").

### Key Features
- Recursive directory scanning
- Extension-based file grouping
- Collision detection and resolution
- Safe filesystem operations with error handling
- Three-phase execution pipeline: Scan → Plan → Execute

### Technology Stack
- **Language**: C++26 (with C++20 modules)
- **Build System**: CMake 4.0.0+ with Ninja
- **Package Manager**: vcpkg
- **Dependencies**: fmt (≥12.1.0), scnlib, ctre

### Entry Point
```bash
./build/binary/debug/Main [directory_path]
```
- Default: Uses current directory if no argument provided
- Output: Organizes files into subdirectories by extension

---

## Codebase Structure

```
File-Janitor/
├── modules/                    # C++20 named modules (.cppm files)
│   ├── filejanitor.cppm       # Primary module aggregating all partitions
│   ├── result_types.cppm      # Error handling types (std::expected)
│   ├── safe_fs.cppm           # Safe filesystem operation wrappers
│   └── fs_ops/                # Core business logic modules
│       ├── fs_ops.cppm        # Domain types (scanned_file, candidate, etc.)
│       ├── movement_plan.cppm # Planning data structures
│       ├── operation_result.cppm  # Operation tracking types
│       ├── scanner/
│       │   └── scanner.cppm   # File collection logic
│       ├── planner/
│       │   └── planner.cppm   # Movement plan generation
│       └── executor/
│           ├── executor.cppm  # Plan execution logic
│           └── execution_report.cppm  # Statistics reporting
│
├── src/                        # Implementation files (.cxx, .cpp)
│   ├── main.cxx               # Entry point
│   ├── safe_fs.cxx            # safe_fs module implementation
│   └── fs_ops/                # Mirrors modules/ structure
│       ├── operation_result.cpp
│       └── scanner/, planner/, executor/
│
├── include/                    # Legacy headers (superseded by modules)
│   └── (mirrored structure)   # Being phased out
│
├── resources/sandbox/          # Test resources
│
├── CMakeLists.txt             # Build configuration
├── CMakePresets.json          # Build presets (debug, release, sanitizers)
├── vcpkg.json                 # Package dependencies
├── vcpkg-configuration.json   # vcpkg registry configuration
├── .clang-format              # Code formatting rules
├── .clang-tidy                # Static analysis configuration
└── .gitignore                 # Git ignore patterns
```

### Module Organization

The project uses **C++20 named modules** with a hierarchical structure:

**Module: `filejanitor`** (Primary aggregator)
- Exports all partitions via `export import :partition_name;`

**Foundation Layer** (no internal dependencies):
- `:result_types` - `Result<T>`, `VoidResult`, `ScanResult`
- `:fs_ops` - `scanned_file`, `candidate`, `operation_status`, etc.
- `:scanner` - `file_collection`, `collect_files()`

**Dependency Layer**:
- `:safe_fs` - Safe wrappers: `safe_scan()`, `exists()`, `rename()`, `create_directories()`
- `:movement_plan` - `movement_plan` container
- `:operation_result` - `operation_result` with factory methods
- `:execution_report` - `execution_report` statistics aggregator

**Aggregation Layer**:
- `:planner` - `generate_plan()` (transforms files into operations)
- `:executor` - `execute_plan()` (executes movement operations)

---

## Architecture & Design Patterns

### Functional Pipeline Architecture

```
Directory Path
      ↓
[SCANNER] → file_collection (binned files + errors)
      ↓
[PLANNER] → movement_plan (operations to execute)
      ↓
[EXECUTOR] → execution_report (statistics)
```

### Key Design Patterns

1. **Monadic Error Handling**
   - `std::expected<T, std::error_code>` for type-safe error propagation
   - Type aliases: `Result<T>`, `VoidResult`, `ScanResult`
   - No exceptions thrown; errors collected and propagated

2. **Generator-Based Iteration**
   - `std::generator<T>` for lazy filesystem traversal
   - Memory-efficient directory scanning

3. **Fluent Interfaces**
   - `execution_report` uses CRTP with deducing `this`
   - Method chaining for readable report building

4. **Ranges & Views**
   - Heavy use of `std::ranges` and `std::views`
   - Composable transformations (filter, transform, group_by)

5. **Module Partitions**
   - Logical separation of concerns
   - Enforced layered dependencies
   - Foundation → Dependency → Aggregation

### Error Handling Philosophy

- **Graceful Degradation**: Errors collected, processing continues
- **No Silent Failures**: All errors tracked and reported
- **Type Safety**: Compile-time enforcement via `std::expected`

---

## Development Workflows

### Initial Setup

```bash
# Clone repository
git clone <repository-url>
cd File-Janitor

# Install dependencies via vcpkg (if not integrated with CMake)
vcpkg install

# Configure build (debug preset)
cmake --preset ninja-debug

# Build
cmake --build build/debug

# Run
./build/binary/debug/Main resources/sandbox
```

### Common Development Cycle

1. **Make Changes** to `.cppm` (module interface) or `.cxx` (implementation)
2. **Format Code**: `clang-format -i <file>`
3. **Check Lint**: `clang-tidy <file> -- <compile_flags>`
4. **Build**: `cmake --build build/debug`
5. **Test Manually**: Run with test directory
6. **Commit**: Follow git conventions (see below)

### Git Workflow

- **Branch Naming**: `claude/<session-id>` for AI-driven development
- **Commits**: Clear, descriptive messages focusing on "why"
- **Pushing**: Always use `git push -u origin <branch-name>`

**Important**: When working on AI-driven tasks, develop on the designated `claude/` branch specified at the start of the session.

### Adding New Modules

When creating new functionality:

1. **Create Module Interface**: `modules/<component>/<name>.cppm`
   - Export public API
   - Import dependencies
   - Use partition syntax: `export module filejanitor:<partition_name>;`

2. **Create Implementation**: `src/<component>/<name>.cxx`
   - Implement module interface
   - Keep logic separate from interface

3. **Update Primary Module**: Add `export import :new_partition;` to `modules/filejanitor.cppm`

4. **Update CMakeLists.txt**: Add to `FILE_SET` in `add_executable()` or library target

5. **Update Dependencies**: If adding vcpkg dependencies, modify `vcpkg.json`

---

## Code Conventions

### Naming Conventions (enforced by clang-tidy)

| Element | Convention | Example |
|---------|-----------|---------|
| **Classes/Structs** | `lowercase_snake_case` | `file_collection`, `movement_plan` |
| **Functions** | `lowercase_snake_case` | `collect_files()`, `generate_plan()` |
| **Variables** | `lowercase_snake_case` | `source_path`, `destination_dir` |
| **Private Members** | Trailing underscore | `data_`, `count_` |
| **Static Members** | `s_` prefix | `s_instance`, `s_counter` |
| **Enums** | `lowercase_snake_case` | `operation_status::success` |
| **Type Aliases** | `PascalCase` or `snake_case` | `Result`, `VoidResult` |

### File Extensions

| Extension | Purpose |
|-----------|---------|
| `.cppm` | C++ module interface files |
| `.cxx` | C++ implementation files |
| `.hxx` | Legacy C++ headers (being phased out) |
| `.cpp` | C++ implementation (alternative to `.cxx`) |

### Code Style (clang-format)

- **Line Length**: 85 characters maximum
- **Indentation**: 2 spaces (no tabs)
- **Braces**: K&R style (open on same line for functions/control flow)
- **Spacing**: Space after control keywords (`if (`, `for (`)
- **Alignment**: Align consecutive assignments, macros, declarations

Example:
```cpp
export module filejanitor:scanner;

import std;
import :result_types;

namespace fs_ops::scanner {
  struct file_collection {
    std::vector<std::filesystem::path> files;
    std::vector<std::error_code>       errors;
  };

  auto collect_files(std::filesystem::path const& directory)
    -> Result<file_collection>;
}
```

### Modern C++ Features to Use

✅ **Preferred**:
- `auto` for type deduction
- Structured bindings: `auto [value, error] = result;`
- Range-based for loops
- `std::expected` for error handling
- `std::generator` for lazy iteration
- `constexpr` for compile-time computation
- `std::ranges` and views
- Designated initializers

❌ **Avoid**:
- Raw pointers (use smart pointers or references)
- Manual memory management (use RAII)
- C-style casts (use `static_cast`, etc.)
- `new`/`delete` (use smart pointers)
- Macros (use constexpr/inline)
- Exceptions (use `std::expected`)

### Error Handling Pattern

```cpp
// Function signature
auto operation() -> Result<ReturnType>;

// Implementation
auto operation() -> Result<ReturnType> {
  auto result = risky_operation();
  if (!result) {
    return std::unexpected(result.error());
  }
  return process(*result);
}

// Usage
auto result = operation();
if (!result) {
  // Handle error: result.error()
  return std::unexpected(result.error());
}
// Use value: *result or result.value()
```

---

## Build System

### CMake Presets

Use presets defined in `CMakePresets.json`:

```bash
# Debug build (no optimization, debug symbols)
cmake --preset ninja-debug
cmake --build build/debug

# Release build (full optimization)
cmake --preset ninja-release
cmake --build build/release

# Release with debug info
cmake --preset ninja-relwithdebinfo
cmake --build build/relwithdebinfo

# Address Sanitizer (detect memory errors)
cmake --preset ninja-asan
cmake --build build/asan

# Undefined Behavior Sanitizer
cmake --preset ninja-ubsan
cmake --build build/ubsan
```

### Build Output Locations

- **Executables**: `build/binary/{config}/Main`
- **Shared Libraries**: `build/{config}/shared_libs/`
- **Static Libraries**: `build/{config}/static_libs/`

### Compiler Flags

The `CompileSettings` interface library enforces:

**Warnings** (treated as errors):
- `-Wall -Wextra -Wpedantic -Werror`
- `-Weffc++ -Wold-style-cast -Woverloaded-virtual`
- `-Wconversion -Wsign-conversion -Wcast-align`
- `-Wunused -Wshadow -Wnon-virtual-dtor`

**Optimization Levels**:
- Debug: `-O0`
- Release: `-O3`
- RelWithDebInfo: `-O2 -g`
- MinSizeRel: `-Os`

### Adding Dependencies

Edit `vcpkg.json`:

```json
{
  "dependencies": [
    "fmt",
    { "name": "scn", "version>=": "4.0.1" },
    "ctre",
    "new-dependency"  // Add here
  ]
}
```

Reconfigure CMake after changes:
```bash
cmake --preset ninja-debug
```

---

## Testing Guidelines

### Current State

**No formal test framework is currently integrated.** Testing is manual.

### Manual Testing Procedure

1. **Prepare Test Directory**:
   ```bash
   cp -r resources/sandbox /tmp/test-dir
   ```

2. **Run File-Janitor**:
   ```bash
   ./build/binary/debug/Main /tmp/test-dir
   ```

3. **Verify Results**:
   - Check files grouped by extension
   - Verify collision handling (numbered suffixes)
   - Confirm no data loss

### Future Testing Recommendations

For AI assistants adding tests:

1. **Framework**: Consider Google Test or Catch2 (add to `vcpkg.json`)
2. **Test Structure**:
   ```
   tests/
   ├── unit/           # Unit tests per module
   ├── integration/    # Pipeline integration tests
   └── fixtures/       # Test data
   ```

3. **Test Naming**: `test_<function>_<scenario>`

4. **Coverage Areas**:
   - Scanner: Directory traversal, error handling
   - Planner: Extension grouping, collision detection
   - Executor: File operations, rollback scenarios

---

## Common Tasks

### Task: Add New File Extension Handling

1. **Location**: No changes needed (dynamic based on discovered extensions)
2. **Collision Handling**: Already implemented in `planner` module

### Task: Add Configuration File Support

1. **Create Config Module**: `modules/config.cppm`
2. **Define Structure**: Use `scnlib` for parsing
3. **Update Main**: Load config before scanning
4. **Update CMake**: Add module to `FILE_SET`

Example:
```cpp
export module filejanitor:config;

import std;
import :result_types;

namespace config {
  struct settings {
    std::filesystem::path target_directory;
    bool                  recursive = true;
    std::vector<std::string> excluded_extensions;
  };

  auto load_config(std::filesystem::path const& config_path)
    -> Result<settings>;
}
```

### Task: Improve Error Reporting

1. **Location**: `modules/fs_ops/executor/execution_report.cppm`
2. **Add Fields**: Extend `execution_report` structure
3. **Update Display**: Modify `main.cxx` output formatting
4. **Maintain Fluent Interface**: Add methods with `auto(this Self&& self)`

### Task: Add Logging

1. **Add Dependency**: `spdlog` to `vcpkg.json`
2. **Create Logger Module**: `modules/logger.cppm`
3. **Integrate**: Import in modules needing logging
4. **Configure**: Add logging levels to configuration

---

## Troubleshooting

### Build Errors

**Problem**: `import std;` not found

**Solution**: Ensure C++20 modules support:
```cmake
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
```
Note: Recent commits replaced `import std;` with GMF includes - this should no longer occur.

---

**Problem**: Module dependency errors

**Solution**: Check module partition ordering in `filejanitor.cppm`:
- Foundation layer first (`:result_types`, `:fs_ops`)
- Dependency layer second
- Aggregation layer last

---

**Problem**: vcpkg dependencies not found

**Solution**:
```bash
# Install vcpkg if not present
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh

# Integrate with CMake
./vcpkg/vcpkg integrate install

# Reconfigure
cmake --preset ninja-debug -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake
```

---

**Problem**: Sanitizer reports errors

**Solution**:
1. Review sanitizer output carefully
2. Common issues:
   - ASAN: Use-after-free, memory leaks (check RAII)
   - UBSAN: Signed overflow, null dereference
3. Fix root cause, don't suppress

---

### Runtime Errors

**Problem**: "Permission denied" when moving files

**Solution**: Check `safe_fs::rename()` error handling - ensure errors are collected, not fatal.

---

**Problem**: Collision numbering doesn't work

**Solution**: Verify `planner::generate_plan()` collision detection logic - ensure destination existence checks before numbering.

---

**Problem**: Files not grouped correctly

**Solution**: Check extension normalization in `planner` - extensions should be lowercase.

---

### Code Quality Issues

**Problem**: clang-tidy warnings on naming

**Solution**: Follow naming conventions:
```bash
# Check configuration
cat .clang-tidy | grep readability-identifier-naming
```

---

**Problem**: clang-format changes differ from expected

**Solution**:
```bash
# Check .clang-format settings
clang-format --style=file --dump-config

# Apply formatting
clang-format -i <file>
```

---

**Problem**: Compiler warnings treated as errors

**Solution**: Fix all warnings - they indicate real issues:
- `-Wconversion`: Narrowing conversions (use explicit casts)
- `-Wsign-conversion`: Signed/unsigned mismatch
- `-Wshadow`: Variable shadowing (rename inner variable)

---

## Best Practices for AI Assistants

### Before Making Changes

1. ✅ **Read Relevant Files**: Use Read tool on files you'll modify
2. ✅ **Understand Module Dependencies**: Check `filejanitor.cppm` exports
3. ✅ **Check Build Configuration**: Review `CMakeLists.txt` if adding files
4. ✅ **Review Recent Commits**: Understand recent architectural changes

### When Implementing Features

1. ✅ **Use TodoWrite**: Track multi-step tasks
2. ✅ **Maintain Module Layering**: Foundation → Dependency → Aggregation
3. ✅ **Follow Error Handling Pattern**: Always use `std::expected`
4. ✅ **Test Incrementally**: Build after each logical change
5. ✅ **Run Sanitizers**: Use ASAN/UBSAN builds to catch errors early

### Code Quality Checklist

Before committing:

- [ ] Code builds without warnings
- [ ] Follows naming conventions
- [ ] Uses modern C++ idioms (auto, ranges, structured bindings)
- [ ] Error handling uses `std::expected`
- [ ] No raw pointers or manual memory management
- [ ] Line length ≤ 85 characters
- [ ] Formatted with clang-format
- [ ] No TODO comments without tracking

### Commit Guidelines

```bash
# Good commit message
git commit -m "Add collision resolution to planner module

Implements numbered suffix strategy when destination files exist.
Uses recursive checking to find first available number."

# Bad commit message (too vague)
git commit -m "Fixed stuff"
```

---

## Module Import Cheat Sheet

```cpp
// In a module interface (.cppm)
export module filejanitor:partition_name;

// Import standard library (via GMF now, not import std;)
#include <filesystem>
#include <vector>
// ... etc.

import std;  // Only if C++23 import std support is working

// Import other partitions
import :result_types;
import :safe_fs;

// Export declarations
export namespace my_namespace {
  auto my_function() -> Result<int>;
}
```

```cpp
// In an implementation file (.cxx)
module filejanitor:partition_name;  // Module purview

import std;
import :dependencies;

// Implementations
auto my_namespace::my_function() -> Result<int> {
  // ...
}
```

```cpp
// In main.cxx or consumer
import filejanitor;  // Imports entire module with all partitions

int main() {
  // Access all exported symbols
  auto result = my_namespace::my_function();
}
```

---

## Quick Reference

### Key Files to Know

| File | Purpose |
|------|---------|
| `modules/filejanitor.cppm` | Primary module aggregator - check partition order |
| `src/main.cxx` | Entry point - execution pipeline |
| `CMakeLists.txt` | Build config - add new files here |
| `CMakePresets.json` | Build presets - use these for configuration |
| `.clang-tidy` | Linting rules - naming conventions |
| `.clang-format` | Formatting rules - style guide |
| `vcpkg.json` | Dependencies - add libraries here |

### Build Commands

```bash
# Configure
cmake --preset ninja-debug

# Build
cmake --build build/debug

# Clean build
rm -rf build/debug && cmake --preset ninja-debug && cmake --build build/debug

# Run
./build/binary/debug/Main [directory]

# Sanitizer build
cmake --preset ninja-asan && cmake --build build/asan
./build/binary/asan/Main [directory]
```

### Essential Imports

```cpp
import filejanitor;           // All partitions
import filejanitor:scanner;   // Just scanner
import filejanitor:planner;   // Just planner
import filejanitor:executor;  // Just executor
import std;                   // Standard library (if supported)
```

---

## Summary

File-Janitor is a modern C++ project showcasing:
- **C++20 Modules** with hierarchical partitions
- **Functional Programming** patterns (ranges, generators, monads)
- **Type-Safe Error Handling** with `std::expected`
- **CMake + vcpkg** for reproducible builds
- **Strict Code Quality** standards (clang-tidy, clang-format, compiler warnings)

When working on this codebase:
1. Respect the module dependency layering
2. Use `std::expected` for all error-prone operations
3. Favor `std::ranges` and views over manual loops
4. Follow strict naming conventions
5. Test with sanitizer builds
6. Keep line length ≤ 85 characters

For questions or clarifications, review recent commits to understand the architectural direction, especially the C++20 modules migration.

---

**Last Updated**: 2026-01-10
**Target Audience**: AI Assistants (Claude, etc.)
**Codebase Version**: C++26 with C++20 Modules

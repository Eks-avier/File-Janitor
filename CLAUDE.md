# CLAUDE.md - AI Assistant Guide for File-Janitor

This document provides comprehensive guidance for AI assistants working with the File-Janitor codebase.

## Table of Contents

1. [Project Overview](#project-overview)
2. [Codebase Architecture](#codebase-architecture)
3. [Build System & Dependencies](#build-system--dependencies)
4. [Development Workflows](#development-workflows)
5. [Code Quality & Conventions](#code-quality--conventions)
6. [Module System](#module-system)
7. [Common Tasks](#common-tasks)
8. [Critical Guidelines](#critical-guidelines)

---

## Project Overview

### Purpose

**File-Janitor** is a modern C++ file organization utility that automatically organizes files in a directory by creating subdirectories based on file extensions. It implements a three-phase pipeline:

1. **Phase 1 (Scanning)**: Recursively scans a target directory to collect all files
2. **Phase 2 (Planning)**: Generates a movement plan, organizing files into extension-based "buckets"
3. **Phase 3 (Execution)**: Executes the movement plan with collision detection and automatic renaming

**Example**: Files like `document.pdf`, `report.pdf`, `image.jpg` become:
- `pdf/document.pdf`
- `pdf/report.pdf`
- `jpg/image.jpg`

### Technology Stack

| Component | Details |
|-----------|---------|
| **Language** | C++20 (targeting C++26 features) |
| **Build System** | CMake 4.0.0+ |
| **Build Generator** | Ninja |
| **Package Manager** | vcpkg (manifest mode) |
| **Module System** | C++20 Named Modules with Global Module Fragment (GMF) |
| **Code Style** | clang-format + clang-tidy |

---

## Codebase Architecture

### Directory Structure

```
File-Janitor/
├── CMakeLists.txt                    # Build configuration
├── CMakePresets.json                 # Build presets (debug, release, asan, ubsan)
├── vcpkg.json                        # Dependency manifest
├── vcpkg-configuration.json          # vcpkg registry configuration
├── .clang-format                     # Code formatting rules
├── .clang-tidy                       # Static analysis configuration
├── .gitignore                        # Git ignore rules
├── README.md                         # Project documentation
│
├── modules/                          # C++20 Module Interface Units (.cppm)
│   ├── filejanitor.cppm              # Primary module (re-exports all partitions)
│   ├── filejanitor-result_types.cppm # Result<T>, VoidResult, ScanResult
│   ├── filejanitor-fs_ops.cppm       # Core data structures
│   ├── filejanitor-scanner.cppm      # Scanner partition
│   ├── filejanitor-safe_fs.cppm      # Safe filesystem operations
│   ├── filejanitor-movement_plan.cppm# Movement plan struct
│   ├── filejanitor-operation_result.cppm# Operation result class
│   ├── filejanitor-execution_report.cppm# Execution report class
│   ├── filejanitor-planner.cppm      # Planner partition
│   └── filejanitor-executor.cppm     # Executor partition
│
├── src/                              # Implementation files
│   ├── main.cxx                      # Entry point
│   ├── safe_fs.cxx                   # Safe filesystem implementation
│   └── fs_ops/
│       ├── operation_result.cpp      # operation_result methods
│       ├── scanner/
│       │   └── scanner.cxx           # collect_files() implementation
│       ├── planner/
│       │   └── planner.cxx           # generate_plan() implementation
│       └── executor/
│           ├── executor.cxx          # execute_plan() implementation
│           └── execution_report.cxx  # execution_report methods
│
├── include/                          # Legacy headers (transitional)
│   ├── result_types.hxx
│   ├── safe_fs.hxx
│   └── fs_ops/
│       ├── fs_ops.hxx
│       ├── movement_plan.hxx
│       ├── operation_result.hxx
│       ├── scanner/
│       │   └── scanner.hxx
│       ├── planner/
│       │   └── planner.hxx
│       └── executor/
│           ├── executor.hxx
│           └── execution_report.hxx
│
└── resources/                        # Test resources
    └── sandbox/                      # Test data directory
```

### Architectural Layers

The project follows a **modular, layered architecture**:

```
┌─────────────────────────────────────────────────────────┐
│              Primary Module (filejanitor)               │
└─────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
┌──────────────┐   ┌──────────────┐   ┌──────────────┐
│   Scanner    │   │   Planner    │   │   Executor   │
│  Partition   │   │  Partition   │   │  Partition   │
└──────────────┘   └──────────────┘   └──────────────┘
        │                   │                   │
        └───────────────────┼───────────────────┘
                            ▼
        ┌───────────────────────────────────────┐
        │      Dependency Layer                 │
        │  ├─ safe_fs                          │
        │  ├─ movement_plan                    │
        │  ├─ operation_result                 │
        │  └─ execution_report                 │
        └───────────────────────────────────────┘
                            │
                            ▼
        ┌───────────────────────────────────────┐
        │      Foundation Layer                 │
        │  ├─ result_types (Result<T>)         │
        │  └─ fs_ops (core data structures)    │
        └───────────────────────────────────────┘
```

**Layer Descriptions:**

1. **Foundation Layer**: Core types and enums (`result_types`, `fs_ops`)
2. **Dependency Layer**: Wrapper types and report structures (`safe_fs`, `movement_plan`, `operation_result`, `execution_report`)
3. **Feature Layer**: High-level algorithms (`scanner`, `planner`, `executor`)
4. **Primary Module**: Unified public interface (`filejanitor`)

### Key Components

#### 1. Safe Filesystem Operations (`safe_fs`)

Wraps `std::filesystem` operations with error-safe wrappers:

```cpp
// Returns std::generator<Result<directory_entry>>
auto safe_scan(const fs::path path) -> std::generator<ScanResult>

// Returns Result<void> (std::expected<void, error_code>)
auto rename(const fs::path& from, const fs::path& to) -> VoidResult
auto create_directories(const fs::path& path) -> VoidResult
```

**Innovation**: Uses C++20 coroutine-based generators for lazy directory iteration with error handling.

#### 2. Scanner (`fs_ops::scanner`)

Location: `src/fs_ops/scanner/scanner.cxx`

```cpp
struct file_collection {
    std::vector<std::filesystem::path> file_bin;
    std::vector<std::error_code> error_bin;
};

auto collect_files(const fs::path& target_directory) -> file_collection
```

**Implementation**: Uses ranges/views with `chunk_by` and filtering to separate valid files from errors.

#### 3. Planner (`fs_ops::planner`)

Location: `src/fs_ops/planner/planner.cxx`

```cpp
auto generate_plan(
    std::vector<fs::path>&& raw_files,
    const fs::path& root_path
) -> movement_plan
```

**Algorithm**:
1. Decorate files with normalized extensions (lowercase)
2. Sort by extension
3. Group consecutive files with same extension
4. Create operations (source → destination with bucket name)

#### 4. Executor (`fs_ops::executor`)

Location: `src/fs_ops/executor/executor.cxx`

```cpp
auto execute_plan(const movement_plan& plan) -> execution_report
```

**Features**:
- **Collision Detection**: Generates numbered alternatives (`file (1).txt`, `file (2).txt`, etc.)
- **Directory Creation**: Automatically creates bucket directories
- **Atomic Reporting**: Accumulates results into `execution_report`

---

## Build System & Dependencies

### CMake Configuration

**Version Requirements:**
- CMake 4.0.0+ (minimum required)
- C++26 Standard (with C++20 features actively used)

**Build Targets:**
1. **Library**: `filejanitor` - Core module library
2. **Executable**: `Main` - Command-line utility

### Dependencies

Managed via vcpkg manifest mode (`vcpkg.json`):

```json
{
  "dependencies": [
    "ctre",
    "scnlib",
    {
      "name": "fmt",
      "version>=": "12.1.0"
    }
  ]
}
```

**Dependency Usage:**
- **fmt**: Modern formatting library (used throughout for `fmt::println`)
- **scnlib**: Lightweight input scanning library
- **ctre**: Compile-time regular expressions

### Build Presets

Available via `CMakePresets.json`:

| Preset | Description | Flags |
|--------|-------------|-------|
| `default-debug` | Standard debug build | `-g -O0` |
| `default-release` | Full optimization | `-O3 -DNDEBUG` |
| `default-relwithdebinfo` | Release with debug info | `-O2 -g -DNDEBUG` |
| `default-minsizerel` | Size-optimized | `-Os -DNDEBUG` |
| `default-asan` | AddressSanitizer | `-g -O1 -fsanitize=address` |
| `default-ubsan` | UBSan | `-g -O1 -fsanitize=undefined` |

**Build Output Directories:**
- Binaries: `build/binary/{preset}/`
- Shared libs: `build/{preset}/shared_libs/`
- Static libs: `build/{preset}/static_libs/`

### Compiler Warnings

**Aggressive warning configuration** (`CMakeLists.txt:16-39`):

**Common (GCC & Clang):**
- `-Wall -Wextra -Wpedantic -pedantic-errors -Werror`
- `-Weffc++ -Wold-style-cast -Woverloaded-virtual`
- `-Wconversion -Wsign-conversion -Wcast-align`
- `-Wshadow -Wnon-virtual-dtor -Wnull-dereference`
- `-Wformat=2 -Wundef -Wmisleading-indentation`

**GCC-specific:**
- `-Wlogical-op -Wuseless-cast`
- `-Wduplicated-cond -Wduplicated-branches`

**Clang-specific:**
- `-Wthread-safety -Wloop-analysis`
- `-Wno-missing-braces`

**IMPORTANT**: All warnings are treated as errors (`-Werror`). Code must compile cleanly.

---

## Development Workflows

### Building the Project

#### Quick Start (Debug Build)

```bash
# Configure and build with debug preset
cmake --workflow --preset default-debug

# Or manually:
cmake --preset default-debug
cmake --build build/debug

# Run the executable
./build/binary/debug/Main [directory]
```

#### Release Build

```bash
cmake --workflow --preset default-release
./build/binary/release/Main [directory]
```

#### Memory Safety Testing

```bash
# AddressSanitizer (detects memory errors)
cmake --workflow --preset default-asan
./build/binary/asan/Main [directory]

# UndefinedBehaviorSanitizer
cmake --workflow --preset default-ubsan
./build/binary/ubsan/Main [directory]
```

### Code Formatting

**Before committing**, always format code:

```bash
find modules src include -name '*.cppm' -o -name '*.cxx' -o -name '*.cpp' -o -name '*.hxx' | xargs clang-format -i
```

**Format Configuration**: `.clang-format`
- Standard: C++20
- Column limit: 85
- Indentation: 2 spaces
- Pointer/reference alignment: Left
- Brace style: Custom (Allman-like with specific rules)

### Static Analysis

**Before committing**, run clang-tidy:

```bash
# Requires compile_commands.json (auto-generated by CMake presets)
clang-tidy -p build/debug src/**/*.cxx modules/**/*.cppm
```

**Configuration**: `.clang-tidy`
- Enabled checks: `bugprone-*`, `cert-*`, `clang-analyzer-*`, `modernize-*`, `readability-*`, `cppcoreguidelines-*`
- Warnings as errors: `bugprone-*`, `cert-*`, `clang-analyzer-*`

### Testing

**Current Status**: No formal test framework configured.

**Manual Testing**:
```bash
# Test with sandbox directory
./build/binary/debug/Main resources/sandbox

# Test with custom directory
./build/binary/debug/Main /path/to/test/directory
```

**Integration Tests**: Test data should be placed in `resources/sandbox/`

---

## Code Quality & Conventions

### Naming Conventions

**From `.clang-tidy` (readability-identifier-naming):**

| Type | Convention | Example |
|------|------------|---------|
| **Classes/Structs** | `lower_case` | `file_collection` |
| **Member Variables** | `lower_case` with `_` suffix | `file_bin_` |
| **Static Members** | `s_` prefix + `lower_case` | `s_instance_` |
| **Global Variables** | `g_` prefix + `lower_case` | `g_config_` |
| **Functions/Methods** | `lower_case` | `collect_files()` |
| **Constants** | `lower_case` | `max_retry_count` |
| **Enums** | `CamelCase` | `OperationStatus` |
| **Enum Constants** | `lower_case` | `success`, `failure` |
| **Type Aliases** | `CamelCase` | `Result`, `VoidResult` |
| **Template Params** | `CamelCase` | `<typename T>` |
| **Namespaces** | `lower_case` | `fs_ops::scanner` |

**Ignored Namespace Abbreviations**: `fs`, `chrono`, `rng`, `vws` (allowed short names)

### File Extensions

- **Module Interface**: `.cppm`
- **Implementation**: `.cxx` (C++ source), `.cpp` (legacy C++ source)
- **Headers**: `.hxx` (legacy, prefer modules)

### Modern C++ Practices

**This project uses cutting-edge C++ features. Follow these patterns:**

#### 1. Use Ranges and Views

```cpp
// GOOD: Functional pipeline
auto result = files
    | std::views::transform(to_scanned_file)
    | std::views::chunk_by(same_extension)
    | std::views::transform(create_operation);

// AVOID: Manual loops when ranges work
```

#### 2. Use std::expected for Error Handling

```cpp
// GOOD: Result types
auto rename(const fs::path& from, const fs::path& to) -> VoidResult;

// Usage:
if (auto result = rename(from, to); !result) {
    fmt::println("Error: {}", result.error().message());
}

// AVOID: Exceptions for expected errors
```

#### 3. Use Coroutine Generators

```cpp
// GOOD: Lazy evaluation with generators
auto safe_scan(const fs::path path) -> std::generator<ScanResult> {
    // Yields results one at a time
}

// AVOID: Eager container creation when streaming is better
```

#### 4. Use Auto with Structured Bindings

```cpp
// GOOD:
auto [files, errors] = collect_files(directory);

// ACCEPTABLE:
const auto& [source, dest, bucket] = operation;

// AVOID: Explicit types when auto works
```

#### 5. Prefer Modules Over Headers

```cpp
// GOOD (in .cxx files):
import filejanitor;

// LEGACY (only for files not yet converted):
#include "fs_ops/scanner.hxx"
```

### Code Complexity Limits

**From `.clang-tidy`:**
- Cognitive complexity: ≤25
- Line count per function: ≤80 lines
- Statement count: ≤50
- Branch count: ≤10
- Parameter count: ≤6

**If you exceed these limits**, consider refactoring into smaller functions.

---

## Module System

### C++20 Modules Architecture

**This project uses C++20 named modules with partitions**, not traditional headers.

#### Global Module Fragment (GMF)

**CRITICAL**: Use GMF instead of `import std`:

```cpp
// CORRECT:
module;

#include <algorithm>
#include <filesystem>
#include <ranges>
#include <vector>

module filejanitor;  // or: module filejanitor:partition_name;

// Implementation...
```

**WHY?** Previous attempts with `import std` caused compiler conflicts. GMF ensures consistency across GCC/Clang.

#### Module Structure

**Primary Module**: `modules/filejanitor.cppm`
```cpp
export module filejanitor;

export import :result_types;
export import :fs_ops;
export import :scanner;
export import :safe_fs;
export import :movement_plan;
export import :operation_result;
export import :execution_report;
export import :planner;
export import :executor;
```

**Partition Example**: `modules/filejanitor-scanner.cppm`
```cpp
export module filejanitor:scanner;

export import :fs_ops;  // Import sibling partition

export namespace fs_ops::scanner
{
  // Declarations...
}
```

#### Module Dependency Graph

```
filejanitor (primary)
├── result_types (foundation)
├── fs_ops (foundation)
│
├── scanner (depends on: fs_ops)
├── safe_fs (depends on: result_types)
├── movement_plan (depends on: fs_ops)
├── operation_result (depends on: fs_ops)
├── execution_report (depends on: fs_ops)
│
├── planner (depends on: movement_plan)
└── executor (depends on: execution_report, movement_plan)
```

**IMPORTANT**: Partitions can import sibling partitions with `export import :partition_name;`

#### Adding a New Module Partition

1. **Create module interface** (`modules/filejanitor-myfeature.cppm`):
```cpp
export module filejanitor:myfeature;

export import :fs_ops;  // Import dependencies

export namespace fs_ops::myfeature
{
  auto my_function() -> void;
}
```

2. **Create implementation** (`src/fs_ops/myfeature/myfeature.cxx`):
```cpp
module;

#include <algorithm>
// Other standard headers...

module filejanitor:myfeature;

namespace fs_ops::myfeature
{
  auto my_function() -> void { /* ... */ }
}
```

3. **Update primary module** (`modules/filejanitor.cppm`):
```cpp
export import :myfeature;
```

4. **Update CMakeLists.txt**:
```cmake
# In FILE_SET CXX_MODULES section:
modules/filejanitor-myfeature.cppm

# In PRIVATE sources section:
src/fs_ops/myfeature/myfeature.cxx
```

---

## Common Tasks

### Adding a New Feature

1. **Identify the layer**: Foundation, Dependency, or Feature?
2. **Create module partition** (see "Adding a New Module Partition" above)
3. **Write implementation** following existing patterns
4. **Update tests** (when test framework is added)
5. **Format code**: `clang-format -i <files>`
6. **Run static analysis**: `clang-tidy -p build/debug <files>`
7. **Build and test**: `cmake --workflow --preset default-debug`

### Modifying Existing Code

1. **Read the module interface** (`modules/filejanitor-*.cppm`)
2. **Locate implementation** (`src/fs_ops/*/`)
3. **Make changes** following existing patterns
4. **Ensure no breaking changes** to exported interfaces
5. **Format and analyze** (clang-format + clang-tidy)
6. **Build with multiple presets** (debug, release, asan)

### Debugging Build Errors

**Common Issues:**

1. **Module dependency errors**:
   - Check `export import` statements in partition files
   - Verify CMakeLists.txt lists modules in dependency order

2. **Compiler warnings as errors**:
   - Fix the warning (preferred)
   - If unavoidable, document why and add exception to CMakeLists.txt

3. **Linker errors with modules**:
   - Ensure `stdc++exp` is linked (CMakeLists.txt:119, 123, 135, 139)
   - Check that implementation files use `module filejanitor:partition;`

4. **vcpkg dependency errors**:
   ```bash
   # Clear vcpkg cache and rebuild
   rm -rf build/
   cmake --workflow --preset default-debug
   ```

### Performance Optimization

**Before optimizing**, profile with:
```bash
# Build with RelWithDebInfo
cmake --workflow --preset default-relwithdebinfo

# Profile with perf/gprof/etc.
perf record ./build/binary/relwithdebinfo/Main [directory]
perf report
```

**Optimization Guidelines:**
- Use `std::views` for lazy evaluation
- Prefer move semantics (`std::move`) for large containers
- Use `std::generator` for streaming data
- Reserve vector capacity when size is known

---

## Critical Guidelines

### DO

- ✅ **Use module imports** (`import filejanitor;`)
- ✅ **Use Global Module Fragment** for standard headers
- ✅ **Follow naming conventions** (clang-tidy rules)
- ✅ **Keep functions small** (≤80 lines)
- ✅ **Use ranges/views** for transformations
- ✅ **Use `std::expected`** for error handling
- ✅ **Format before committing** (clang-format)
- ✅ **Run static analysis** (clang-tidy)
- ✅ **Test with sanitizers** (ASan, UBSan)
- ✅ **Write clear commit messages**
- ✅ **Preserve existing architecture** (layered modules)

### DON'T

- ❌ **DON'T use `import std`** (use GMF instead)
- ❌ **DON'T add new headers** (prefer modules)
- ❌ **DON'T use exceptions** for expected errors
- ❌ **DON'T use raw pointers** (use smart pointers/references)
- ❌ **DON'T ignore compiler warnings**
- ❌ **DON'T skip code formatting**
- ❌ **DON'T create circular module dependencies**
- ❌ **DON'T commit without building** (at least debug preset)
- ❌ **DON'T use `using namespace` in headers/modules** (only in .cxx files)
- ❌ **DON'T over-engineer** (keep solutions simple)

### When to Ask for Clarification

- Uncertain about which layer a feature belongs to
- Need to add a new third-party dependency
- Considering breaking changes to public interfaces
- Unsure about module dependency ordering
- Need to deviate from established patterns

---

## Git Workflow

### Branch Naming

- Feature branches: `feature/description`
- Bugfixes: `fix/description`
- Refactoring: `refactor/description`
- Claude Code branches: `claude/claude-md-<session-id>`

### Commit Messages

**Format:**
```
<type>: <short summary>

<optional detailed description>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `refactor`: Code refactoring
- `docs`: Documentation changes
- `style`: Code formatting
- `perf`: Performance improvements
- `test`: Test additions/modifications
- `build`: Build system changes

**Examples:**
```
feat: Add collision detection to executor

Implements numbered suffix generation (file (1).txt) when
destination files already exist. Supports up to 100 candidates.

refactor: Decompose fs_ops into scanner/planner/executor

Splits monolithic fs_ops.cxx into three focused components with
clear separation of concerns.
```

### Pre-Commit Checklist

1. ☐ Code formatted with clang-format
2. ☐ No clang-tidy warnings
3. ☐ Builds successfully (debug + release)
4. ☐ No compiler warnings
5. ☐ Tested manually (if applicable)
6. ☐ Commit message follows conventions

---

## Troubleshooting

### Build Fails with Module Errors

**Symptom**: "module interface not found" or "duplicate module definition"

**Solution**:
1. Clean build directory: `rm -rf build/`
2. Verify module order in CMakeLists.txt matches dependency graph
3. Check that partitions use `module filejanitor:partition;` correctly
4. Ensure primary module re-exports all partitions

### Sanitizer Reports Issues

**AddressSanitizer (ASan):**
- Memory leaks: Check for missing destructors
- Use-after-free: Check object lifetimes
- Heap buffer overflow: Check array bounds

**UBSan:**
- Signed integer overflow: Use unsigned or checked arithmetic
- Null dereference: Add null checks
- Unaligned access: Review struct packing

### vcpkg Issues

**Symptom**: "Package not found" or "Version conflict"

**Solution**:
```bash
# Update vcpkg
cd /path/to/vcpkg
git pull
./bootstrap-vcpkg.sh

# Clear cache
rm -rf build/
cmake --workflow --preset default-debug
```

---

## Additional Resources

### File References

- Build configuration: `CMakeLists.txt`
- Build presets: `CMakePresets.json`
- Dependencies: `vcpkg.json`
- Code style: `.clang-format`
- Static analysis: `.clang-tidy`
- Git ignore: `.gitignore`

### Key Source Files

- Entry point: `src/main.cxx:10-90`
- Scanner: `src/fs_ops/scanner/scanner.cxx`
- Planner: `src/fs_ops/planner/planner.cxx`
- Executor: `src/fs_ops/executor/executor.cxx`
- Safe FS: `src/safe_fs.cxx`

### Module Interfaces

- Primary module: `modules/filejanitor.cppm`
- All partitions: `modules/filejanitor-*.cppm`

---

## Questions?

If you encounter situations not covered in this guide:

1. **Check existing code** for similar patterns
2. **Review commit history** (`git log`) for context
3. **Examine module interfaces** for API contracts
4. **Ask for clarification** before making significant changes

---

**Last Updated**: 2026-01-10
**Version**: 1.0.0

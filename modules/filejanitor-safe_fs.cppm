// Module partition: safe_fs
// Exports: safe_scan(), exists(), rename(), create_directories()
// Depends on: result_types
module;

#include <expected>
#include <filesystem>
#include <generator>
#include <system_error>

// Include headers in global module fragment
#include "result_types.hxx"
#include "safe_fs.hxx"

export module filejanitor:safe_fs;

// Re-export dependency partition
export import :result_types;

// Re-export functions from safe_fs namespace
export namespace safe_fs {
    using ::safe_fs::safe_scan;
    using ::safe_fs::exists;
    using ::safe_fs::rename;
    using ::safe_fs::create_directories;
}

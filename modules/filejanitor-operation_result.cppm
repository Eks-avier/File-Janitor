// Module partition: operation_result
// Exports: operation_result class
// Depends on: fs_ops
module;

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <system_error>
#include <utility>

// Include headers in global module fragment
#include "fs_ops/fs_ops.hxx"
#include "fs_ops/operation_result.hxx"

export module filejanitor:operation_result;

// Re-export dependency partition
export import :fs_ops;

// Re-export operation_result class
export namespace fs_ops {
    using ::fs_ops::operation_result;
}

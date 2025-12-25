// Module partition: movement_plan
// Exports: movement_plan struct
// Depends on: fs_ops
module;

#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

// Include headers in global module fragment
#include "fs_ops/fs_ops.hxx"
#include "fs_ops/movement_plan.hxx"

export module filejanitor:movement_plan;

// Re-export dependency partition
export import :fs_ops;

// Re-export movement_plan struct
export namespace fs_ops {
    using ::fs_ops::movement_plan;
}

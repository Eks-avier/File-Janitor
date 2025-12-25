// Module partition: planner
// Exports: generate_plan()
// Depends on: movement_plan (transitively: fs_ops)
module;

#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

// Include headers in global module fragment
#include "fs_ops/fs_ops.hxx"
#include "fs_ops/movement_plan.hxx"
#include "fs_ops/planner/planner.hxx"

export module filejanitor:planner;

// Re-export dependency partition (provides movement_plan and fs_ops types)
export import :movement_plan;

// Re-export planner function
export namespace fs_ops::planner {
    using ::fs_ops::planner::generate_plan;
}

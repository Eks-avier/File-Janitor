// Module partition: executor
// Exports: execute_plan()
// Depends on: execution_report, movement_plan (transitively: fs_ops)
module;

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

// Include headers in global module fragment
#include "fs_ops/fs_ops.hxx"
#include "fs_ops/movement_plan.hxx"
#include "fs_ops/executor/execution_report.hxx"
#include "fs_ops/executor/executor.hxx"

export module filejanitor:executor;

// Re-export dependency partitions
export import :execution_report;
export import :movement_plan;

// Re-export executor function
export namespace fs_ops::executor {
    using ::fs_ops::executor::execute_plan;
}

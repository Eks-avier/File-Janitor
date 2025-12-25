// Module partition: execution_report
// Exports: execution_report class
// Depends on: fs_ops
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
#include "fs_ops/executor/execution_report.hxx"

export module filejanitor:execution_report;

// Re-export dependency partition
export import :fs_ops;

// Re-export execution_report class
export namespace fs_ops::executor {
    using ::fs_ops::executor::execution_report;
}

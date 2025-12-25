// Module partition: fs_ops
// Exports: scanned_file, candidate, operation_status, successful_operation, failed_operation
module;

#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>

// Include header in global module fragment
#include "fs_ops/fs_ops.hxx"

export module filejanitor:fs_ops;

// Re-export types from fs_ops namespace
export namespace fs_ops {
    using ::fs_ops::scanned_file;
    using ::fs_ops::candidate;
    using ::fs_ops::operation_status;
    using ::fs_ops::successful_operation;
    using ::fs_ops::failed_operation;
}

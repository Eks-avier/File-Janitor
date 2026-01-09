// Module partition: movement_plan
// Exports: movement_plan struct
// Depends on: fs_ops
module;

#include <vector>

export module filejanitor:movement_plan;

// Re-export dependency partition
export import :fs_ops;

// Define and export movement_plan struct
export namespace fs_ops
{
  struct movement_plan
  {
    std::vector<successful_operation> operations;
  };
} // namespace fs_ops

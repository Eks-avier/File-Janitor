// Module partition: planner
// Exports: generate_plan()
// Depends on: movement_plan (transitively: fs_ops)
module;

#include <filesystem>
#include <vector>

export module filejanitor:planner;

// Re-export dependency partition (provides movement_plan and fs_ops types)
export import :movement_plan;

// Declare and export planner function
export namespace fs_ops::planner
{
  [[nodiscard]] auto generate_plan(
              std::vector<std::filesystem::path>&& raw_files,
              const std::filesystem::path&         root_path
  ) -> movement_plan;
}

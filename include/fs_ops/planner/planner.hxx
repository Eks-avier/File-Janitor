//
// Created by Xavier on 12/25/2025.
//

#ifndef FILEJANITOR_PLANNER_HXX
#define FILEJANITOR_PLANNER_HXX

#include "movement_plan.hxx"
#include <filesystem>
#include <vector>

namespace fs_ops::planner
{
  [[nodiscard]] auto generate_plan(
              std::vector<std::filesystem::path>&& raw_files,
              const std::filesystem::path&         root_path
  ) -> movement_plan;
}

#endif // FILEJANITOR_PLANNER_HXX

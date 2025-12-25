//
// Created by Xavier on 12/25/2025.
//

#ifndef FILEJANITOR_MOVEMENT_PLAN_HXX
#define FILEJANITOR_MOVEMENT_PLAN_HXX

#include "fs_ops.hxx"
#include <vector>

namespace fs_ops
{
  struct movement_plan
  {
    std::vector<fs_ops::successful_operation> operations;
  };
} // namespace fs_ops

#endif // FILEJANITOR_MOVEMENT_PLAN_HXX

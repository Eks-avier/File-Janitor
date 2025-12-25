//
// Created by Xavier on 12/25/2025.
//

#ifndef FILEJANITOR_EXECUTOR_HXX
#define FILEJANITOR_EXECUTOR_HXX

#include "executor/execution_report.hxx";
#include "movement_plan.hxx"

namespace fs_ops::executor
{

  [[maybe_unused]] auto execute_plan(const movement_plan& plan) -> execution_report;
} // namespace fs_ops::executor

#endif // FILEJANITOR_EXECUTOR_HXX

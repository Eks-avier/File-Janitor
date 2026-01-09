// Module partition: executor
// Exports: execute_plan()
// Depends on: execution_report, movement_plan (transitively: fs_ops)
export module filejanitor:executor;

// Re-export dependency partitions
export import :execution_report;
export import :movement_plan;

// Declare and export executor function
export namespace fs_ops::executor {
  [[nodiscard]] auto execute_plan(const movement_plan& plan) -> execution_report;
}

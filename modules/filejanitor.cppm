// Primary module interface: filejanitor
// Re-exports all partitions for unified import
export module filejanitor;

// Foundation partitions
export import :result_types;
export import :fs_ops;
export import :scanner;

// Dependency layer partitions
export import :safe_fs;
export import :movement_plan;
export import :operation_result;
export import :execution_report;

// Aggregation partitions
export import :planner;
export import :executor;

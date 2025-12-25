// Module partition: result_types
// Exports: Result<T>, VoidResult, ScanResult
module;

#include <expected>
#include <filesystem>
#include <system_error>
#include <vector>

// Include header in global module fragment for definitions
#include "result_types.hxx"

export module filejanitor:result_types;

// Re-export type aliases (must redeclare for templates)
export template <typename T>
using Result = std::expected<T, std::error_code>;

export using VoidResult = Result<void>;
export using ScanResult = Result<std::filesystem::directory_entry>;

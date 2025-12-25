// Module partition: result_types
// Exports: Result<T>, VoidResult, ScanResult
module;

#include <expected>
#include <filesystem>
#include <system_error>

export module filejanitor:result_types;

// Define and export type aliases
export template <typename T>
using Result = std::expected<T, std::error_code>;

export using VoidResult = Result<void>;
export using ScanResult = Result<std::filesystem::directory_entry>;

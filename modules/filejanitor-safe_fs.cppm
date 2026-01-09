// Module partition: safe_fs
// Exports: safe_scan(), exists(), rename(), create_directories()
// Depends on: result_types
module;

#include <filesystem>
#include <generator>

export module filejanitor:safe_fs;

// Re-export dependency partition
export import :result_types;

// Declare and export functions
export namespace safe_fs {
  [[nodiscard]] auto safe_scan(std::filesystem::path path) -> std::generator<ScanResult>;
  [[nodiscard]] auto exists(const std::filesystem::path& path) noexcept -> bool;
  [[nodiscard]] auto rename(const std::filesystem::path& from, const std::filesystem::path& to) -> VoidResult;
  [[nodiscard]] auto create_directories(const std::filesystem::path& path) -> VoidResult;
} // namespace safe_fs

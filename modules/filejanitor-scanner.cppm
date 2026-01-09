// Module partition: scanner
// Exports: file_collection, collect_files()
module;

#include <algorithm>
#include <expected>
#include <filesystem>
#include <span>
#include <system_error>
#include <vector>

export module filejanitor:scanner;

// Define and export types directly in the module
export namespace fs_ops::scanner {
  struct file_collection {
    std::vector<std::filesystem::path> file_bin;
    std::vector<std::error_code>       error_bin;
  };

  [[nodiscard]] auto collect_files(const std::filesystem::path& target_directory)
              -> file_collection;
} // namespace fs_ops::scanner

export namespace fs_ops::new_scanner {
  class scanner {
  public:
    using scanned_files   = std::vector<std::filesystem::path>;
    using scanned_errors  = std::vector<std::error_code>;
    using collection_view = std::span<const std::filesystem::path>;

    struct scanned_result {
      scanned_files  files;
      scanned_errors errors;
    };

    enum class scanner_state { target_not_found, no_files, no_errors };

    using result = std::expected<scanned_result, scanner_state>;

    static auto collect_from(const std::filesystem::path& target) noexcept -> scanner;

    // TODO: Refactor into chainable member functions

    auto has_errors() const noexcept -> bool;
    auto has_files() const noexcept -> bool;

    auto errors() const noexcept -> scanned_errors;

    auto view() const noexcept -> collection_view;
    auto own() const noexcept -> scanned_files;

  private:
    using scan_result = std::expected<std::filesystem::directory_entry, std::error_code>;

    explicit scanner(scanned_result result) noexcept;

    static auto exists(const std::filesystem::path& target) noexcept -> bool;
    static auto scan_from(const std::filesystem::path& target) noexcept -> std::vector<scan_result>;
    static auto partition(std::vector<scan_result>&& results) noexcept -> std::vector<scan_result>;
    static auto make_result(std::vector<scan_result>&& results) noexcept -> scanned_result;

    scanned_result m_result;
  };
} // namespace fs_ops::new_scanner

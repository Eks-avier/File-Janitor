// Module partition: scanner
// Exports: file_collection, collect_files()
module;

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

  [[nodiscard]] auto collect_files(const std::filesystem::path& target_directory) -> file_collection;
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

    enum class scanner_state { target_not_found, no_files };

    template <typename T>
    using result_type = std::expected<T, scanner_state>;

    static auto collect_from(const std::filesystem::path& target) noexcept -> result_type<scanner>;

    auto view() const noexcept -> collection_view;
    auto own() const noexcept -> scanned_files;

  private:
    using scan_result = std::expected<std::filesystem::directory_entry, std::error_code>;
    friend class result_type<scanner>;

    struct key_t {};
    static constexpr key_t passkey{};

    explicit scanner(scanned_result result) noexcept;
    explicit scanner(key_t, scanned_result result) noexcept;

    static auto exists(const std::filesystem::path& target) noexcept -> result_type<std::filesystem::path>;
    static auto scan(const std::filesystem::path& target) noexcept -> result_type<std::vector<scan_result>>;
    static auto to_partitioned(std::vector<scan_result> raw) noexcept -> std::vector<scan_result>;
    static auto to_scanned(std::vector<scan_result> partitioned) noexcept -> scanned_result;

    scanned_result m_result;
  };
} // namespace fs_ops::new_scanner

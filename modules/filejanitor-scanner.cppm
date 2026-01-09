// Module partition: scanner
// Exports: file_collection, collect_files()
module;

#include <expected>
#include <filesystem>
#include <system_error>
#include <vector>

export module filejanitor:scanner;

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
    using scanned_files  = std::vector<std::filesystem::path>;
    using scanned_errors = std::vector<std::error_code>;

    struct output_t {
      scanned_files  files;
      scanned_errors errors;
    };

    enum class state_t { target_not_found, no_files };

    template <typename T>
    using result_t = std::expected<T, state_t>;

    static auto collect_from(const std::filesystem::path& target) noexcept -> result_t<scanner>;

    [[nodiscard]] auto take() && noexcept -> output_t;
    [[nodiscard]] auto view() const noexcept -> const output_t&;

  private:
    using scan_result = std::expected<std::filesystem::directory_entry, std::error_code>;

    friend class result_t<scanner>;

    struct key_t {};
    static constexpr key_t key{};

    explicit scanner(key_t, output_t result) noexcept;

    static auto exists(const std::filesystem::path& target) noexcept -> result_t<std::filesystem::path>;
    static auto scan(const std::filesystem::path& target) noexcept -> result_t<std::vector<scan_result>>;
    static auto to_partitioned(std::vector<scan_result> raw) noexcept -> std::vector<scan_result>;
    static auto to_scanned(const std::vector<scan_result>& partitioned) noexcept -> output_t;

    output_t m_output;
  };
} // namespace fs_ops::new_scanner

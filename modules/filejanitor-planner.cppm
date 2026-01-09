// Module partition: planner
// Exports: generate_plan()
// Depends on: movement_plan (transitively: fs_ops)
module;

#include <expected>
#include <filesystem>
#include <vector>

export module filejanitor:planner;

export import :movement_plan;

// Declare and export planner function
export namespace fs_ops::planner {
  [[nodiscard]] auto
  generate_plan(std::vector<std::filesystem::path>&& raw_files, const std::filesystem::path& root_path)
              -> movement_plan;
}

export namespace fs_ops::new_planner {

  struct decorated_file {
    std::filesystem::path root;
    std::filesystem::path path;
    std::string           extension;
  };

  struct operation_t {
    std::filesystem::path from;
    std::filesystem::path to;
  };

  struct planned_operation {
    operation_t operation;
    std::string bucket_name;
  };

  class planner;

  class planner_builder {
  public:
    using raw_data_t = std::vector<std::filesystem::path>;

    enum class state_t {};

    using output_t        = std::optional<planner>;
    using planner_input_t = std::vector<planned_operation>;

    static auto from(std::pair<std::filesystem::path, raw_data_t> data) -> output_t;

    // TODO: Refine builder for future features

  private:
    planner_builder(std::filesystem::path root, raw_data_t data) noexcept;

    static auto to_decorated(planner_builder&& builder) -> std::vector<decorated_file>;
    static auto to_sorted(std::vector<decorated_file>&& decorated) -> std::vector<decorated_file>;
    static auto to_chunked(std::vector<decorated_file>&& sorted) -> std::vector<std::vector<decorated_file>>;
    static auto to_planned(std::vector<std::vector<decorated_file>>&& chunked) -> planner_input_t;
    static auto to_planner(planner_input_t&& planned) -> planner;

    // TODO: No invariants? Or set up for future invariants? Configs, strategies, etc.
    std::filesystem::path m_root{};
    raw_data_t            m_files{};
  };

  class planner {
  public:
    using output_t = std::vector<planned_operation>;

  private:
    friend class planner_builder;

    struct key_t {};
    static constexpr key_t key;

  public:
    explicit planner(key_t, output_t output) noexcept;

    auto view() const& noexcept -> const output_t&;
    auto take() && noexcept -> output_t;

  private:
    output_t m_output{};
  };
} // namespace fs_ops::new_planner

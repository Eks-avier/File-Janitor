//
// Created by Xavier on 12/25/2025.
//

module;

// Standard headers in GMF
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <ranges>
#include <string>
#include <vector>

module filejanitor;

namespace fs  = std::filesystem;
namespace rng = std::ranges;
namespace vws = std::views;

using namespace fs_ops;

namespace {
  auto to_lowercase(std::string_view string) -> std::string {
    return string | vws::transform([](const unsigned char& c) { return std::tolower(c); }) | rng::to<std::string>();
  }

  auto normalize_extension(const fs::path& path) -> std::string {
    return not path.has_extension() ? std::string{} : to_lowercase(path.extension().string());
  }

  auto decorate_with_extensions(std::vector<fs::path> raw_files) -> std::vector<scanned_file> {
    return std::move(raw_files)
           | vws::transform([](const fs::path& p) {
               return scanned_file{.path{p}, .extension{normalize_extension(p.extension().string())}};
             })
           | rng::to<std::vector>();
  }

  auto sort_by_extension(std::vector<scanned_file>&& raw_files) -> std::vector<scanned_file> {
    auto to_sort{std::move(raw_files)};
    rng::sort(to_sort, {}, &scanned_file::extension);
    return to_sort;
  }

  auto make_bucket(const scanned_file& file) -> std::string {
    return file.extension.empty() ? "no_extension" : file.extension.substr(1);
  }

  auto generate(const std::vector<scanned_file>& sorted, const fs::path& r_path) -> movement_plan {
    auto compare_extensions{[](const auto& a, const auto& b) { return a.extension == b.extension; }};

    auto empty_chunks{[](const auto& chunk) { return not chunk.empty(); }};

    auto make_move_plans{[&r_path](const auto& chunk) {
      return chunk
             | vws::transform([&r_path, b_name{make_bucket(chunk.front())}](const auto& file) {
                 return successful_operation{
                      .source{file.path},
                      .destination{r_path / b_name / file.path.filename()},
                      .bucket_name{b_name}
                 };
               })
             | rng::to<std::vector<successful_operation>>();
    }};

    return {
         sorted
         | vws::chunk_by(compare_extensions)
         | vws::filter(empty_chunks)
         | vws::transform(make_move_plans)
         | vws::join
         | rng::to<std::vector>()
    };
  }
} // namespace

// TODO: To be deprecated
namespace fs_ops::planner {
  auto generate_plan(std::vector<fs::path>&& raw_files, const fs::path& root_path) -> movement_plan {
    // 1. Decorate the raw files with extensions
    // 2. Sort the decorated files
    // 3. Generate plans from sorted files
    return generate(sort_by_extension(decorate_with_extensions(std::move(raw_files))), root_path);
  }
} // namespace fs_ops::planner

namespace fs_ops::new_planner {

  namespace /* HELPERS */ {
    auto make_decorated(const fs::path& root, std::pair<fs::path, fs::path> data) -> decorated_file {
      auto&& [path, extension]{data};
      return {
           .root{root},
           .path{std::move(path)},
           .extension{
                extension.string()
                | vws::transform([](const unsigned char& c) { return std::tolower(c); })
                | rng::to<std::string>()
           }
      };
    }

    auto make_operation(const std::tuple<fs::path, std::string, fs::path>& data) -> operation_t {
      const auto& [root, category, path]{data};
      return {.from{path}, .to{root / category / path.filename()}};
    }

    auto make_planned_operation(decorated_file file) -> planned_operation {
      auto&& [root, path, extension]{file};
      const auto category = extension.empty() ? "no_extension" : extension.substr(1);
      return {.operation{make_operation({root, category, path})}, .bucket_name{category}};
    }

    auto make_planned_operations(std::vector<decorated_file> chunk) noexcept -> std::vector<planned_operation> {
      return chunk
             | vws::as_rvalue
             | vws::transform([](const decorated_file& file) { return make_planned_operation(file); })
             | rng::to<std::vector>();
    }
  } // namespace

  auto planner_builder::from(std::pair<std::filesystem::path, raw_data_t> data) -> output_t {
    // TODO: Write the pipeline here
  }

  planner_builder::planner_builder(std::filesystem::path root, raw_data_t data) noexcept
      : m_root{std::move(root)}
      , m_data{std::move(data)} {}

  auto planner_builder::to_decorated(planner_builder builder) -> std::vector<decorated_file> {
    const auto& root = builder.m_root;
    auto        data = std::move(builder.m_data);
    return builder.m_data
           | vws::as_rvalue
           | vws::transform([&root](const fs::path& p) { return make_decorated(root, {p, p.extension()}); })
           | rng::to<std::vector>();
  }
  auto planner_builder::to_sorted(std::vector<decorated_file> decorated) -> std::vector<decorated_file> {
    auto to_sort{std::move(decorated)};
    rng::sort(to_sort, {}, &decorated_file::extension);
    return to_sort;
  }

  auto planner_builder::to_chunked(std::vector<decorated_file> sorted) -> std::vector<std::vector<decorated_file>> {
    return sorted
           | vws::as_rvalue
           | vws::chunk_by([](const auto& a, const auto& b) { return a.extension == b.extension; })
           | vws::filter([](const auto& chunk) { return not chunk.empty(); })
           | rng::to<std::vector<std::vector<decorated_file>>>();
  }
  auto planner_builder::to_planned(std::vector<std::vector<decorated_file>> chunked) -> std::vector<planned_operation> {
    return chunked
           | vws::as_rvalue
           | vws::transform([](const auto& chunk) { return make_planned_operations(chunk); })
           | vws::join
           | rng::to<std::vector>();
  }

  auto planner_builder::to_planner(std::vector<planned_operation> planned) -> planner {
    return planner{planner::key, std::move(planned)};
  }

  planner::planner(key_t, output_t output) noexcept
      : m_output{std::move(output)} {}
} // namespace fs_ops::new_planner

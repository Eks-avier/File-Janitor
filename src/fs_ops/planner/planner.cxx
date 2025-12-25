//
// Created by Xavier on 12/25/2025.
//

import std;

#include "fs_ops.hxx"
#include "planner/planner.hxx"

namespace fs  = std::filesystem;
namespace rng = std::ranges;
namespace vws = std::views;

using namespace fs_ops;

namespace
{
  auto normalize_extension(const fs::path& path) -> std::string
  {
    return not path.has_extension()
                       ? std::string{}
                       : path.extension().string()
                                     | vws::transform([](const unsigned char c) {
                                         return std::tolower(c);
                                       })
                                     | rng::to<std::string>();
  }

  auto decorate_with_extensions(std::vector<fs::path> raw_files)
              -> std::vector<scanned_file>
  {
    return std::move(raw_files)
           | vws::transform([](const fs::path& p) {
               return scanned_file{.path{p}, .extension{normalize_extension(p)}};
             })
           | rng::to<std::vector>();
  }

  auto sort_by_extension(std::vector<scanned_file>&& raw_files)
              -> std::vector<scanned_file>
  {
    return [to_sort = std::move(raw_files)] mutable {
      rng::sort(to_sort, {}, &scanned_file::extension);
      return to_sort;
    }();
  }

  auto make_bucket(const scanned_file& file) -> std::string
  {
    return file.extension.empty() ? "no_extension" : file.extension.substr(1);
  }

  auto generate(const std::vector<scanned_file>& sorted, const fs::path& r_path)
              -> movement_plan
  {
    auto compare_extensions{[](const auto& a, const auto& b) {
      return a.extension == b.extension;
    }};

    auto empty_chunks{[](const auto& chunk) { return not chunk.empty(); }};

    auto make_move_plans{[&r_path](const auto& chunk) {
      return chunk
             | vws::transform([&r_path, b_name{make_bucket(chunk.front())}](
                                          const auto& file
                              ) {
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

namespace fs_ops::planner
{
  // TODO: Can be given a fluent interface or a monadic style
  auto generate_plan(std::vector<fs::path>&& raw_files, const fs::path& root_path)
              -> movement_plan
  {
    // 1. Decorate the raw files with extensions
    // 2. Sort the decorated files
    // 3. Generate plans from sorted files
    return generate(
                sort_by_extension(decorate_with_extensions(std::move(raw_files))),
                root_path
    );
  }
} // namespace fs_ops::planner

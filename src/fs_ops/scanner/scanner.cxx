//
// Created by Xavier on 12/25/2025.
//

import std;

#include "safe_fs.hxx"
#include "result_types.hxx"
#include "scanner/scanner.hxx"

namespace fs  = std::filesystem;
namespace rng = std::ranges;
namespace vws = std::views;

namespace
{
  auto get_raw_results(const fs::path& target_directory) -> std::vector<ScanResult>
  {
    return rng::to<std::vector>(safe_fs::safe_scan(target_directory));
  }

  auto split(std::vector<ScanResult>& raw_results) -> auto
  {
    return rng::partition(raw_results, [](const auto& result) {
      return result.has_value();
    });
  }
} // namespace

namespace fs_ops::scanner
{
  auto collect_files(const fs::path& target_directory) -> file_collection
  {
    // 1. Collects raw results
    // 2. Partitions raw results
    // 3. Collect valid files
    // 4. Collect errors
    return [raw_results{get_raw_results(target_directory)}] mutable {
      return [raw_begin{raw_results.begin()},
              split_point{split(raw_results)}] -> file_collection {
        return {
             .file_bin{
                  std::span{raw_begin, split_point.begin()}
                  | vws::filter([](const ScanResult& r) {
                      return r->is_regular_file();
                    })
                  | vws::transform([](const ScanResult& r) { return r->path(); })
                  | rng::to<std::vector>()
             },
             .error_bin{
                  std::span{split_point}
                  | vws::transform([](const ScanResult& r) { return r.error(); })
                  | rng::to<std::vector>()
             }
        };
      }();
    }();
  }
} // namespace fs_ops::scanner

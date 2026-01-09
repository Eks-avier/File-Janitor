//
// Created by Xavier on 12/25/2025.
//

module;

#include <algorithm>
#include <filesystem>
#include <ranges>
#include <span>
#include <vector>

module filejanitor;

namespace fs  = std::filesystem;
namespace rng = std::ranges;
namespace vws = std::views;

namespace {
  auto get_raw_results(const fs::path& target_directory) -> std::vector<ScanResult> {
    return rng::to<std::vector>(safe_fs::safe_scan(target_directory));
  }

  auto split(std::vector<ScanResult>& raw_results) -> auto {
    return rng::partition(raw_results, [](const auto& result) { return result.has_value(); });
  }
} // namespace

namespace fs_ops::scanner {
  auto collect_files(const fs::path& target_directory) -> file_collection {
    // 1. Collects raw results
    // 2. Partitions raw results
    // 3. Collect valid files
    // 4. Collect errors
    return [raw_results{get_raw_results(target_directory)}] mutable {
      return [raw_begin{raw_results.begin()}, split_point{split(raw_results)}] -> file_collection {
        return {
             .file_bin{
                  std::span{raw_begin, split_point.begin()}
                  | vws::filter([](const ScanResult& r) { return r->is_regular_file(); })
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

namespace fs_ops::new_scanner {
  auto scanner::collect_from(const fs::path& target) noexcept -> result_t<scanner> {
    const auto result{exists(target).and_then(scan).transform(to_partitioned).transform(to_scanned)};
    return result.has_value() ? result_t<scanner>{std::in_place, key, *result}
                              : result_t<scanner>{std::unexpect, result.error()};
  }

  auto scanner::take() && noexcept -> output_t { return std::move(m_output); }
  auto scanner::view() const noexcept -> const output_t& { return m_output; }

  scanner::scanner(output_t result) noexcept
      : m_output{std::move(result)} {}

  scanner::scanner(key_t, output_t result) noexcept
      : m_output{std::move(result)} {}

  auto scanner::exists(const fs::path& target) noexcept -> result_t<fs::path> {
    return safe_fs::exists(target) ? result_t<fs::path>{std::in_place, target}
                                   : result_t<fs::path>{std::unexpect, state_t::target_not_found};
  }

  auto scanner::scan(const fs::path& target) noexcept -> result_t<std::vector<scan_result>> {
    const auto result = rng::to<std::vector>(safe_fs::safe_scan(target));
    return result.empty() ? result_t<std::vector<scan_result>>{std::unexpect, state_t::no_files}
                          : result_t<std::vector<scan_result>>{std::in_place, result};
  }

  auto scanner::to_partitioned(std::vector<scan_result> raw) noexcept -> std::vector<scan_result> {
    auto to_partition{std::move(raw)};
    rng::partition(to_partition, [](const auto& result) { return result.has_value(); });
    return to_partition;
  }

  auto scanner::to_scanned(const std::vector<scan_result>& partitioned) noexcept -> output_t {
    const auto partition_point{rng::partition_point(partitioned, [](const scan_result& r) { return r.has_value(); })};
    return {
         .files{
              std::span{partitioned.begin(), partition_point}
              | vws::filter([](const scan_result& r) { return r->is_regular_file(); })
              | vws::transform([](const scan_result& r) { return r->path(); })
              | rng::to<std::vector>()
         },
         .errors{
              std::span{partition_point, partitioned.end()}
              | vws::transform([](const scan_result& r) { return r.error(); })
              | rng::to<std::vector>()
         }
    };
  }

} // namespace fs_ops::new_scanner

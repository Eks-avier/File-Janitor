//
// Created by Xavier on 12/25/2025.
//

module;

// Standard headers in GMF
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
        return {.file_bin{std::span{raw_begin, split_point.begin()}
                          | vws::filter([](const ScanResult& r) { return r->is_regular_file(); })
                          | vws::transform([](const ScanResult& r) { return r->path(); })
                          | rng::to<std::vector>()},
                .error_bin{std::span{split_point}
                           | vws::transform([](const ScanResult& r) { return r.error(); })
                           | rng::to<std::vector>()}};
      }();
    }();
  }
} // namespace fs_ops::scanner

namespace fs_ops::new_scanner {

  auto scanner::collect_from(const fs::path& target) noexcept -> scanner {
    auto raw_files{scan_from(target)}; // TODO: Make expected, then chain operations.
    auto partitioned{partition(std::move(raw_files))};
    return scanner{make_result(std::move(partitioned))};
  }

  auto scanner::has_errors() const noexcept -> bool { return m_result.errors.empty(); }
  auto scanner::has_files() const noexcept -> bool { return m_result.files.empty(); }
  auto scanner::errors() const noexcept -> scanned_errors { return m_result.errors; }
  auto scanner::view() const noexcept -> collection_view { return m_result.files; }
  auto scanner::own() const noexcept -> scanned_files { return m_result.files; }

  scanner::scanner(scanned_result result) noexcept
      : m_result{std::move(result)} {}

  auto scanner::exists(const fs::path& target) noexcept -> bool { return safe_fs::exists(target); }

  auto scanner::scan_from(const fs::path& target) noexcept -> std::vector<scan_result> {
    return rng::to<std::vector>(safe_fs::safe_scan(target));
  }

  auto scanner::partition(std::vector<scan_result>&& results) noexcept -> std::vector<scan_result> {
    auto to_partition{std::move(results)};
    auto predicate{[](const auto& result) { return result.has_value(); }};
    rng::partition(to_partition, predicate);
    return to_partition;
  }

  auto scanner::make_result(std::vector<scan_result>&& results) noexcept -> scanned_result {
    const auto predicate{[](const auto& result) { return result.has_value(); }};
    const auto partition_point{rng::partition_point(results, predicate)};

    return {.files{std::span{results.cbegin(), partition_point}
                   | vws::filter([](const scan_result& r) { return r->is_regular_file(); })
                   | vws::transform([](const scan_result& r) { return r->path(); })
                   | rng::to<std::vector>()},
            .errors{std::span{partition_point, results.cend()}
                    | vws::transform([](const scan_result& r) { return r.error(); })
                    | rng::to<std::vector>()}};
  }

} // namespace fs_ops::new_scanner

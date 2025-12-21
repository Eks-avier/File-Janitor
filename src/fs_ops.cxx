//
// Created by Xavier on 12/16/2025.
//

#include "fs_ops.hxx"
#include "safe_fs.hxx"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <filesystem>
#include <fmt/format.h>
#include <ranges>
#include <string>

namespace fs  = std::filesystem;
namespace vws = std::views;
namespace rng = std::ranges;

namespace fs_ops
{
  namespace
  {
    auto get_raw_results(const fs::path& target_directory) -> std::vector<ScanResult>
    {
      return rng::to<std::vector>(safe_fs::safe_scan(target_directory));
    }

    auto split(std::vector<ScanResult>& raw_results) -> auto
    {
      return rng::partition(raw_results, [](const auto& result) { return result.has_value(); });
    }

    auto normalize_extension(const fs::path& path) -> std::string
    {
      return not path.has_extension() ? std::string{} : [&path] -> std::string {
        return path.extension().string()
               | vws::transform([](const unsigned char c) { return std::tolower(c); })
               | rng::to<std::string>();
      }();
    }

    struct ScannedFile
    {
      fs::path    path;
      std::string extension;
    };

    auto make_bucket_name(const ScannedFile& file) -> std::string
    {
      return file.extension.empty() ? "no_extension" : file.extension.substr(1);
    }

    auto sort_by_extension(std::vector<ScannedFile>&& raw_files) -> std::vector<ScannedFile>
    {
      return [to_sort = std::move(raw_files)] mutable {
        rng::sort(to_sort, {}, &ScannedFile::extension);
        return to_sort;
      }();
    }

    auto decorate_with_extensions(std::vector<fs::path> raw_files) -> std::vector<ScannedFile>
    {
      return std::move(raw_files)
             | vws::transform([](const fs::path& p) -> ScannedFile {
                 return { .path{ p }, .extension{ normalize_extension(p) } };
               })
             | rng::to<std::vector>();
    }

    auto generate(const std::vector<ScannedFile>& sorted_files, const fs::path& root_path)
            -> MovementPlan
    {
      auto compare_extensions{ [](const auto& a, const auto& b) {
        return a.extension == b.extension;
      } };

      auto empty_chunks{ [](const auto& chunk) { return not chunk.empty(); } };

      auto make_move_plans{ [&root_path](const auto& chunk) {
        return chunk
               | vws::transform([&root_path,
                                 bucket_name = make_bucket_name(chunk.front())](const auto& file) {
                   return Operation{ .source{ file.path },
                                     .destination{ root_path / bucket_name / file.path.filename() },
                                     .bucket_name{ bucket_name } };
                 })
               | rng::to<std::vector<Operation>>();
      } };

      return { sorted_files
               | vws::chunk_by(compare_extensions)
               | vws::filter(empty_chunks)
               | vws::transform(make_move_plans)
               | vws::join
               | rng::to<std::vector>() };
    }

    auto does_target_exist(const fs::path& target) -> std::optional<fs::path>
    {
      return not safe_fs::exists(target) ? std::optional{ target } : std::nullopt;
    }

    struct Candidate
    {
      fs::path    parent;
      std::string stem;
      std::string extension;
    };

    auto build_candidate(const fs::path& target) -> Candidate
    {
      return { .parent{ target.parent_path() },
               .stem{ target.stem().string() },
               .extension{ target.extension().string() } };
    }

    auto make_candidate_path(const Candidate& candidate, int idx) -> fs::path
    {
      return [idx, parent{ candidate.parent }, ext{ candidate.extension }, stem{ candidate.stem }] {
        return fs::path{ parent / fmt::format("{} ({}){}", stem, idx, ext) };
      }();
    }

    auto make_candidate_paths(const Candidate& c) -> std::vector<fs::path>
    {
      constexpr int max_candidate_index{ 100 };
      return vws::iota(1, max_candidate_index)
             | vws::transform([&c](const int idx) { return make_candidate_path(c, idx); })
             | rng::to<std::vector<fs::path>>();
    }

    auto find_valid_candidate(const std::vector<fs::path>& c_paths) -> std::optional<fs::path>
    {
      auto predicate_to_satisfy{ [](const fs::path& c) { return not safe_fs::exists(c); } };

      return [end = c_paths.end(),
              it  = rng::find_if(c_paths, predicate_to_satisfy)] -> std::optional<fs::path> {
        return (it != end) ? std::optional{ *it } : std::nullopt;
      }();
    }

    auto make_valid_candidate(const fs::path& target) -> std::optional<fs::path>
    {
      return find_valid_candidate(make_candidate_paths(build_candidate(target)));
    }

    auto resolve_collision(const fs::path& target) -> fs::path
    {
      return does_target_exist(target)
              .or_else([&target] { return make_valid_candidate(target); })
              .value_or(target);
    }

    auto ensure_directory(const fs::path& dir) -> VoidResult
    {
      return safe_fs::create_directories(dir);
    }

    auto execute(const Operation& op, ExecutionReport&& report) -> ExecutionReport
    {
      auto result = [&op, rep = std::move(report)] {
        return ensure_directory(op.destination.parent_path())
                .and_then([src = op.source, dst = resolve_collision(op.destination)] {
                  return safe_fs::rename(src, dst);
                })
                .transform([r = std::move(rep)] mutable {
                  return std::move(r).with_processed().with_success();
                })
                .or_else([r = std::move(rep), src = op.source, dst = op.destination](
                                 const auto e) mutable
                                 -> std::expected<ExecutionReport, std::error_code> {
                  return std::move(r).with_processed().with_failure(FailedOperation{
                          .source = src, .intended_destination = dst, .error = e });
                })
                .value();
      }();

      return op.source == op.destination ? std::move(report).with_processed() : result;
    }

  } // namespace

  auto get_extension(const fs::path& path) -> std::optional<fs::path>
  {
    return path.has_extension() ? std::optional{ path.extension() } : std::optional<fs::path>{};
  }

  // TODO: I see an opportunity to use chained functions
  auto collect_files(const fs::path& target_directory) -> FileCollection
  {
    return [raw_results{ get_raw_results(target_directory) }] mutable -> FileCollection {
      return [raw_begin{ raw_results.begin() },
              split_point{ split(raw_results) }] -> FileCollection {
        return { .file_bin{ std::span{ raw_begin, split_point.begin() }
                            | vws::transform([](const ScanResult& r) { return r->path(); })
                            | rng::to<std::vector>() },

                 .error_bin{ std::span{ split_point }
                             | vws::transform([](const ScanResult& r) { return r.error(); })
                             | rng::to<std::vector>() } };
      }();
    }();
  }

  auto generate_plan(std::vector<fs::path>&& raw_files, const fs::path& root_path) -> MovementPlan
  {
    return generate(sort_by_extension(decorate_with_extensions(std::move(raw_files))), root_path);
  }

  auto execute_plan(const MovementPlan& plan) -> ExecutionReport
  {
    return rng::fold_left(plan.operations,
                          ExecutionReport{},
                          [](ExecutionReport report, const Operation& operation) {
                            return execute(operation, std::move(report));
                          });
  }

} // namespace fs_ops

//
// Created by Xavier on 12/25/2025.
//

import std;

import "fmt/format.h";

import "safe_fs.hxx";

import "executor/executor.hxx";

namespace fs  = std::filesystem;
namespace vws = std::views;
namespace rng = std::ranges;

using namespace fs_ops;
using namespace fs_ops::executor;

namespace
{
  auto does_target_exist(const fs::path& target) -> std::optional<fs::path>
  {
    return not safe_fs::exists(target) ? std::optional{target} : std::nullopt;
  }

  auto build_candidate(const fs::path& target) -> candidate
  {
    return {
         .parent{target.parent_path()},
         .stem{target.stem().string()},
         .extension{target.extension().string()}
    };
  }

  auto make_candidate_path(const candidate& c, int idx) -> fs::path
  {
    return [idx, par{c.parent}, ext{c.extension}, stem{c.stem}] {
      return fs::path{par / fmt::format("{} ({}){}", stem, idx, ext)};
    }();
  }

  auto make_candidate_paths(const candidate& c) -> std::vector<fs::path>
  {
    constexpr int max_candidate_index{100};
    return vws::iota(1, max_candidate_index)
           | vws::transform([&c](const int idx) {
               return make_candidate_path(c, idx);
             })
           | rng::to<std::vector<fs::path>>();
  }

  auto find_valid_candidate(const std::vector<fs::path>& c_paths)
              -> std::optional<fs::path>
  {
    auto predicate_to_satisfy{[](const fs::path& c) {
      return not safe_fs::exists(c);
    }};

    return [end{c_paths.end()},
            result_it{rng::find_if(c_paths, predicate_to_satisfy)}]
           -> std::optional<fs::path> {
      return result_it != end ? std::optional{*result_it} : std::nullopt;
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

  auto perform_move(const successful_operation& op) -> VoidResult
  {
    return ensure_directory(op.destination.parent_path()).and_then([&op] {
      return safe_fs::rename(op.source, resolve_collision(op.destination));
    });
  }

  auto process_operation(const successful_operation& op) -> operation_result
  {
    if ( op.source == op.destination )
      return operation_result::create_skipped();

    return [result{perform_move(op)}, &op] {
      return result.has_value()
                         ? operation_result::create_success()
                         : operation_result::create_failure(op, result.error());
    }();
  }

  auto accumulate_reports(execution_report&& report, const operation_result& result)
              -> execution_report
  {
    using enum operation_status;

    switch ( result.status() )
    {
    case success:
      return std::move(report).with_processed().with_success().finalize();
    case failure:
      return std::move(report)
                  .with_processed()
                  .with_failure(result.failure()->get())
                  .finalize();
    case skipped: return std::move(report).with_processed().finalize();
    }

    std::unreachable();
  }
} // namespace

namespace fs_ops::executor
{
  auto execute_plan(const movement_plan& plan) -> execution_report
  {
    return rng::fold_left(
                plan.operations | vws::transform(process_operation),
                execution_report::start(),
                accumulate_reports
    );
  }
} // namespace fs_ops::executor

//
// Created by Xavier on 12/16/2025.
//

#ifndef FILEJANITOR_FS_OPS_HXX
#define FILEJANITOR_FS_OPS_HXX

#include "result_types.hxx"
#include <filesystem>
#include <optional>
#include <vector>

struct FileCollection
{
  std::vector<std::filesystem::path> file_bin{};
  std::vector<std::error_code>       error_bin{};
};

struct Operation
{
  std::filesystem::path source;
  std::filesystem::path destination;
  std::string           bucket_name;
};

struct FailedOperation
{
  std::filesystem::path source;
  std::filesystem::path intended_destination;
  std::error_code       error;
};

struct MovementPlan
{
  std::vector<Operation> operations;
};

struct ExecutionReport
{
  std::vector<FailedOperation> failures;
  int                          processed_count{ 0 };
  int                          success_count{ 0 };

  auto with_processed(this ExecutionReport&& self) -> ExecutionReport
  {
    self.processed_count++;
    return std::move(self);
  }

  auto with_success(this ExecutionReport&& self) -> ExecutionReport
  {
    self.success_count++;
    return std::move(self);
  }

  auto with_failure(this ExecutionReport&& self, const FailedOperation& error) -> ExecutionReport
  {
    self.failures.push_back(error);
    return std::move(self);
  }
};

namespace fs_ops
{
  [[nodiscard]] auto get_extension(const std::filesystem::path& path) -> std::optional<std::filesystem::path>;

  [[nodiscard]] auto collect_files(const std::filesystem::path& target_directory) -> FileCollection;

  [[nodiscard]] auto generate_plan(std::vector<std::filesystem::path>&& raw_files,
                                   const std::filesystem::path&         root_path) -> MovementPlan;

  // NEW: Executes the plan safely, handling collisions and permissions
  [[maybe_unused]] auto execute_plan(const MovementPlan& plan) -> ExecutionReport;

} // namespace fs_ops

#endif // FILEJANITOR_FS_OPS_HXX

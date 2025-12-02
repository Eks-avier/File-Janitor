#include <file_janitor/display.hxx>
#include <file_janitor/filesystem_ops.hxx>
#include <file_janitor/organization.hxx>
#include <filesystem>
#include <fmt/base.h>

auto main() -> int
{
  using namespace file_janitor;
  namespace fs = std::filesystem;

  fs::path target_dir = "./resources/test_files";

  // Validate directory
  if ( !is_valid_directory(target_dir) )
  {
    fmt::println(
          stderr, "Error: '{}' is not a valid directory", target_dir.string());
    return 1;
  }

  // Collect files
  auto files_result = collect_files_by_extension(target_dir);
  if ( !files_result )
  {
    fmt::println(stderr, "Error: {}", files_result.error().message());
    return 1;
  }

  // Create organization plan (Sprint 2!)
  auto plan_result = create_organization_plan(*files_result, target_dir);
  if ( !plan_result )
  {
    fmt::println(stderr, "Error: {}", plan_result.error().message());
    return 1;
  }

  // Display the plan (dry-run preview)
  display_organization_plan(*plan_result);

  return 0;
}

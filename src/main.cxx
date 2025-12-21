#include <filesystem>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

// THIRD PARTY
#include "fs_ops.hxx"
#include <fmt/base.h>

using namespace std::string_view_literals;
using namespace std::string_literals;

namespace fs  = std::filesystem;
namespace vws = std::views;
namespace rng = std::ranges;

namespace
{
  [[maybe_unused]] [[nodiscard]] auto filename_as_string(const fs::path& entry) -> std::string
  {
    return entry.filename().string();
  }

  [[maybe_unused]] [[nodiscard]] auto extension_as_string(const fs::path& path) -> std::string
  {
    return path.extension().string();
  }

} // namespace

auto main() -> int
{
  constexpr auto test_directory{
    "C:/Users/Xavier/Desktop/Xavier_CPP/CPP Serious Projects/FileJanitor/resources/test_files"sv
  };

  if ( !fs::exists(test_directory) )
  {
    fmt::println(stderr, "Directory not found: {}", test_directory);
    return 1;
  }

  // --- PHASE 1: COLLECT ---
  fmt::println("--- PHASE 1: SCANNING ---");
  auto [files, errors] = fs_ops::collect_files(test_directory);

  fmt::println("Found {} files.", files.size());

  if ( not errors.empty() )
  {
    fmt::println("Encountered {} errors during scan.", errors.size());
  }

  if ( files.empty() )
  {
    fmt::println("No files to organize. Exiting.");
    return 0;
  }

  // --- PHASE 2: PLAN ---
  fmt::println("\n--- PHASE 2: PLANNING ---");
  const auto result = fs_ops::generate_plan(std::move(files), test_directory);

  fmt::println("Generated {} operations.", result.operations.size());

  for ( const auto& [source, destination, bucket_name] : result.operations )
  {
    fmt::println("[PLAN] {} -> {} (Bucket: {})",
                 source.filename().string(),
                 destination.string(),
                 bucket_name);
  }

  // --- PHASE 3: EXECUTE ---
  // In a real CLI, we would ask for confirmation here.
  fmt::println("\n--- PHASE 3: EXECUTION ---");

  const auto [failures, processed_count, success_count] = fs_ops::execute_plan(result);

  fmt::println("Execution Complete.");
  fmt::println("  Processed: {}", processed_count);
  fmt::println("  Success:   {}", success_count);
  fmt::println("  Failures:  {}", failures.size());

  if ( not failures.empty() )
  {
    fmt::println("\n[!] Errors:");
    for ( const auto& [source, intended_destination, error] : failures )
    {
      fmt::println("  - Failed to move '{}' -> '{}': {}",
                   source.filename().string(),
                   intended_destination.string(),
                   error.message());
    }
  }
  return 0;
}

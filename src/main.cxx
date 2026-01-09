// Standard headers BEFORE fmt (fmt includes std headers internally)
#include <filesystem>
#include <fmt/ostream.h>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

import filejanitor;

using namespace std::string_view_literals;
using namespace std::string_literals;

namespace fs = std::filesystem;

namespace {
  auto deprecated_flow(const fs::path& test_directory) noexcept -> int {
    if ( not fs::exists(test_directory) ) {
      fmt::println(stderr, "Directory not found: {}", test_directory.string());
      return 1;
    }

    // --- PHASE 1: COLLECT ---
    fmt::println("--- PHASE 1: SCANNING ---");
    auto [files, errors] = fs_ops::scanner::collect_files(test_directory);

    fmt::println("Found {} files.", files.size());

    if ( not errors.empty() ) {
      fmt::println("Encountered {} errors during scan.", errors.size());
    }

    if ( files.empty() ) {
      fmt::println("No files to organize. Exiting.");
      return 0;
    }

    // --- PHASE 2: PLAN ---
    fmt::println("\n--- PHASE 2: PLANNING ---");
    const auto result = fs_ops::planner::generate_plan(std::move(files), test_directory);

    fmt::println("Generated {} operations.", result.operations.size());

    for ( const auto& [source, destination, bucket_name] : result.operations ) {
      fmt::println("[PLAN] {} -> {} (Bucket: {})", source.filename().string(), destination.string(), bucket_name);
    }

    // --- PHASE 3: EXECUTE ---
    // In a real CLI, we would ask for confirmation here.
    fmt::println("\n--- PHASE 3: EXECUTION ---");

    const auto report = fs_ops::executor::execute_plan(result);

    fmt::println("Execution Complete.");
    fmt::println("  Processed: {}", report.processed_count());
    fmt::println("  Success:   {}", report.success_count());
    fmt::println("  Failures:  {}", report.failure_count());
    fmt::println("  Skipped:   {}", report.skipped_count());

    if ( report.failure_count() > 0 ) {
      fmt::println("\n[!] Errors:");
      for ( const auto& [source, intended_destination, error] : report.failures() ) {
        fmt::println(
                    "  - Failed to move '{}' -> '{}': {}",
                    source.filename().string(),
                    intended_destination.string(),
                    error.message()
        );
      }
    }
  }

  auto revised_flow(fs::path dir) noexcept -> int {
    if ( not fs::exists(dir) ) {
      fmt::println(stderr, "Directory not found: {}", dir.string());
      return 1;
    }

    // --- PHASE 1: COLLECT ---
    fmt::println("--- PHASE 1: SCANNING ---");
    const auto scanner = fs_ops::new_scanner::scanner::collect_from(std::move(dir));
    const auto& [files, errors]{scanner->view()};

    if ( not errors.empty() ) {
      fmt::println("Encountered {} errors during scan.", errors.size());
    }

    if ( files.empty() ) {
      fmt::println("No files to organize. Exiting.");
      return 0;
    }

    // --- PHASE 2: PLAN ---
    fmt::println("\n--- PHASE 2: PLANNING ---");
  }
} // namespace

auto main(const int argc, char* argv[]) -> int {
  // Default to current directory if no arg provided, or use the first argument
  const auto raw_path       = (argc > 1) ? std::string_view{argv[1]} : "."sv;
  const auto test_directory = fs::absolute(raw_path);

  revised_flow(test_directory);

  return 0;
}

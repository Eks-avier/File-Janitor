// Module partition: scanner
// Exports: file_collection, collect_files()
module;

#include <filesystem>
#include <system_error>
#include <vector>

export module filejanitor:scanner;

// Define and export types directly in the module
export namespace fs_ops::scanner {
    struct file_collection {
        std::vector<std::filesystem::path> file_bin;
        std::vector<std::error_code>       error_bin;
    };

    [[nodiscard]] auto collect_files(const std::filesystem::path& target_directory)
        -> file_collection;
}

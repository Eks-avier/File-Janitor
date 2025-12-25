//
// Created by Xavier on 12/25/2025.
//

#ifndef FILEJANITOR_SCANNER_HXX
#define FILEJANITOR_SCANNER_HXX

#include <filesystem>
#include <system_error>
#include <vector>

namespace fs_ops::scanner
{
  struct file_collection
  {
    std::vector<std::filesystem::path> file_bin;
    std::vector<std::error_code>       error_bin;
  };

  [[nodiscard]] auto collect_files(const std::filesystem::path& target_directory)
              -> file_collection;
} // namespace fs_ops::scanner

#endif // FILEJANITOR_SCANNER_HXX

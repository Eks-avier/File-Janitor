//
// Created by Xavier on 12/16/2025.
//

#ifndef FILEJANITOR_SAFE_FS_HXX
#define FILEJANITOR_SAFE_FS_HXX

#include "result_types.hxx"
#include <filesystem>
#include <generator>

namespace safe_fs
{
  [[nodiscard]] auto safe_scan(std::filesystem::path path)
              -> std::generator<ScanResult>;

  [[nodiscard]] auto exists(const std::filesystem::path& path) noexcept -> bool;

  [[nodiscard]] auto
  rename(const std::filesystem::path& from, const std::filesystem::path& to)
              -> VoidResult;

  [[nodiscard]] auto create_directories(const std::filesystem::path& path)
              -> VoidResult;

} // namespace safe_fs

#endif // FILEJANITOR_SAFE_FS_HXX

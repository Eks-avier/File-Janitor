#ifndef FILE_JANITOR_FILESYSTEM_OPS_HXX
#define FILE_JANITOR_FILESYSTEM_OPS_HXX

#include <file_janitor/constants.hxx>
#include <file_janitor/errors.hxx>
#include <file_janitor/types.hxx>

#include <expected>
#include <filesystem>
#include <vector>

namespace file_janitor
{
  [[maybe_unused]] auto is_valid_directory(const std::filesystem::path& directory) -> bool;

  [[maybe_unused]] auto collect_files_by_extension(const std::filesystem::path& dir)
        -> std::expected<FilesByExtension, DirectoryScanError>;

  [[nodiscard]] auto
  get_existing_folders(const std::filesystem::path& target_directory) noexcept
        -> std::expected<std::vector<std::string>, DirectoryScanError>;

} // namespace file_janitor

#endif // FILE_JANITOR_FILESYSTEM_OPS_HXX

#ifndef FILE_JANITOR_ORGANIZATION_HXX
#define FILE_JANITOR_ORGANIZATION_HXX

#include <file_janitor/constants.hxx>
#include <file_janitor/errors.hxx>
#include <file_janitor/filesystem_ops.hxx>
#include <file_janitor/types.hxx>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace file_janitor
{
  [[nodiscard]] constexpr auto parse_int(std::string_view view) noexcept
          -> std::optional<int>;

  auto get_collision_suffix(
        std::string_view                base_name,
        const std::vector<std::string>& existing_folder_names) noexcept
        -> std::optional<int>;

  [[maybe_unused]] auto create_folder_groups(const FilesByExtension& files)
        -> FolderGroups;

  auto collect_folder_files(const ExtensionList&    extensions,
                            const FilesByExtension& data_source)
        -> std::vector<std::filesystem::path>;

  auto create_folders(FolderGroups&&                  folder_groups,
                      const std::vector<std::string>& existing_folders,
                      const FilesByExtension&         files_source)
        -> std::vector<Folder>;

  auto create_organization_plan(const FilesByExtension&     files,
                                const std::filesystem::path& target_directory)
        -> std::expected<std::vector<Folder>, DirectoryScanError>;

} // namespace file_janitor

#endif // FILE_JANITOR_ORGANIZATION_HXX

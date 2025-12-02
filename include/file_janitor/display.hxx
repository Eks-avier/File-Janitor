#ifndef FILE_JANITOR_DISPLAY_HXX
#define FILE_JANITOR_DISPLAY_HXX

#include <file_janitor/constants.hxx>
#include <file_janitor/types.hxx>

#include <cstddef>
#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

namespace file_janitor
{
  class FolderDisplayContext
  {
  public:
    explicit FolderDisplayContext(std::span<const Folder> folders);

    auto print_title() -> FolderDisplayContext&;
    auto print_collisions() -> FolderDisplayContext&;
    auto print_category(FolderCategory category) -> FolderDisplayContext&;
    auto print_summary() -> void;

  private:
    static void print_separator();
    static void print_single_folder(const Folder& folder);

    std::span<const Folder> folders_;

    static constexpr std::size_t      separator_width_{ 50 };
    static constexpr std::string_view separator_char_{ "═" };
  };

  auto display_organization_plan(const std::vector<Folder>& folders) -> void;

  [[maybe_unused]] auto display_results(const FilesByExtension& files) noexcept
        -> void;

} // namespace file_janitor

#endif // FILE_JANITOR_DISPLAY_HXX

#ifndef FILE_JANITOR_TYPES_HXX
#define FILE_JANITOR_TYPES_HXX

#include <cstdint>
#include <filesystem>
#include <flat_map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace file_janitor
{
  template <typename... Types>
  struct Overloaded : Types...
  {
    using Types::operator()...;
  };

  using FilesByExtension
        = std::flat_map<std::string, std::vector<std::filesystem::path>, std::less<>>;

  using ExtensionList
        = std::variant<std::vector<std::string_view>, std::vector<std::string>>;

  using FolderGroups = std::flat_map<std::string, ExtensionList, std::less<>>;

  enum class FolderCategory : std::uint8_t
  {
    regular,
    others,
    no_extension
  };

  struct Folder
  {
    std::string                          base_name;
    std::optional<int>                   collision_suffix;
    FolderCategory                       category;
    ExtensionList                        extensions;
    std::vector<std::filesystem::path>   files;
  };

} // namespace file_janitor

#endif // FILE_JANITOR_TYPES_HXX

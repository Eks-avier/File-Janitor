#ifndef FILE_JANITOR_CONSTANTS_HXX
#define FILE_JANITOR_CONSTANTS_HXX

#include <file_janitor/types.hxx>

#include <algorithm>
#include <array>
#include <fmt/color.h>
#include <fmt/format.h>
#include <optional>
#include <string_view>
#include <utility>

namespace file_janitor
{
  namespace constants
  {
    using namespace std::string_view_literals;

    constexpr auto g_NO_EXTENSION{ "~Empty"sv };
    constexpr auto g_OTHERS_FOLDER_NAME{ "Others"sv };

    constexpr auto g_KNOWN_EXTENSIONS{
      std::array{ std::pair{ ".png"sv, "Images"sv },
                 std::pair{ ".jpg"sv, "Images"sv },
                 std::pair{ ".svg"sv, "Images"sv },
                 std::pair{ ".docx"sv, "Documents"sv },
                 std::pair{ ".pdf"sv, "Documents"sv },
                 std::pair{ ".cpp"sv, "Source Code"sv },
                 std::pair{ ".py"sv, "Source Code"sv },
                 std::pair{ ".json"sv, "Data Files"sv },
                 std::pair{ ".csv"sv, "Data Files"sv },
                 std::pair{ ".md"sv, "Markdown"sv },
                 std::pair{ ".txt"sv, "Text Files"sv },
                 std::pair{ ".pptx"sv, "Presentations"sv },
                 std::pair{ ".mp3"sv, "Audio"sv },
                 std::pair{ ".wav"sv, "Audio"sv },
                 std::pair{ g_NO_EXTENSION, "No Extension"sv } }
    };

  } // namespace constants

  constexpr auto get_folder_name(std::string_view extension) -> std::string_view
  {
    using namespace constants;

    const auto* iterator{ std::ranges::find_if(
          g_KNOWN_EXTENSIONS,
          [extension](const auto& pair) -> auto
          { return pair.first == extension; }) };

    return iterator != g_KNOWN_EXTENSIONS.end() ? iterator->second
                                                : g_OTHERS_FOLDER_NAME;
  }

  constexpr auto get_known_extension(std::string_view requested_extension)
        -> std::optional<std::string_view>
  {
    using namespace constants;

    const auto* iterator{ std::ranges::find_if(
          g_KNOWN_EXTENSIONS,
          [requested_extension](const auto& pair) -> auto
          { return pair.first == requested_extension; }) };

    return iterator != g_KNOWN_EXTENSIONS.end()
                 ? std::make_optional(iterator->first)
                 : std::nullopt;
  }

  constexpr auto get_folder_style(FolderCategory category) -> fmt::text_style
  {
    using enum FolderCategory;

    switch ( category )
    {
    case others      : return fg(fmt::color::yellow) | fmt::emphasis::bold;
    case no_extension: return fg(fmt::color::magenta) | fmt::emphasis::bold;
    default          : return fg(fmt::color::cyan) | fmt::emphasis::bold;
    }
  }

  constexpr auto get_folder_category(std::string_view folder_name)
        -> FolderCategory
  {
    using namespace constants;
    using enum FolderCategory;

    if ( folder_name == g_OTHERS_FOLDER_NAME )
    {
      return others;
    }

    if ( folder_name == g_NO_EXTENSION )
    {
      return no_extension;
    }

    return regular;
  }

  [[maybe_unused]] inline auto get_resolved_name(const Folder& folder) -> std::string
  {
    return folder.collision_suffix
                 ? fmt::format(
                         "{} ({})", folder.base_name, *folder.collision_suffix)
                 : folder.base_name;
  }

} // namespace file_janitor

#endif // FILE_JANITOR_CONSTANTS_HXX

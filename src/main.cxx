#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <flat_map>
#include <fmt/base.h>
#include <fmt/format.h>
#include <format>
#include <functional>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace
{
  namespace fs = std::filesystem;

  using FilesByExtension =
        std::flat_map<std::string, std::vector<fs::path>, std::less<>>;

  namespace constants // g_ for global constants
  {
    using namespace std::string_view_literals;

    constexpr auto g_NO_EXTENSION{ "~Empty"sv };

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
          [extension](const auto& pair) { return pair.first == extension; }) };

    return iterator != g_KNOWN_EXTENSIONS.end() ? iterator->second : "Others"sv;
  }

  enum class FileOrganizerError : std::uint8_t
  {
    directory_iterator_failed
  };

  constexpr auto format_as(FileOrganizerError error) -> std::string_view
  {
    using enum FileOrganizerError;

    switch ( error )
    {
    case directory_iterator_failed: return "Directory iterator failed";
    }

    return "Unknown error";
  }

  class DirectoryScanError
  {
  public:
    DirectoryScanError(FileOrganizerError category, std::error_code code)
        : category_(category)
        , code_(code)
    {}

    [[nodiscard]] auto category() const -> FileOrganizerError
    {
      return category_;
    }

    [[nodiscard]] auto message() const -> std::string
    {
      return fmt::format("{}: {}", category_, code_.message());
    }

  private:
    FileOrganizerError category_;
    std::error_code    code_;
  };

  auto is_valid_directory(const fs::path& directory) -> bool
  {
    return fs::exists(directory) and fs::is_directory(directory);
  }

  auto collect_files_by_extension(const fs::path& dir)
        -> std::expected<FilesByExtension, DirectoryScanError>
  {
    using enum FileOrganizerError;
    using namespace constants;

    FilesByExtension container{};
    std::error_code  code{};

    for ( auto iterator{ fs::directory_iterator(dir, code) };
          iterator != fs::directory_iterator();
          iterator.increment(code) )
    {
      if ( code )
      {
        return std::unexpected{
          DirectoryScanError{ directory_iterator_failed, code }
        };
      }

      const auto& entry{ *iterator };

      if ( not entry.is_regular_file() )
      {
        continue;
      }

      auto extension{ entry.path().extension().string() };
      auto key = extension.empty() ? std::string{ g_NO_EXTENSION } : extension;
      container[key].emplace_back(entry.path());
    }

    if ( code )
    {
      return std::unexpected{
        DirectoryScanError{ directory_iterator_failed, code }
      };
    }

    return container;
  }

  auto display_results(const FilesByExtension& files) -> void
  {
    using namespace constants;
    namespace vws = std::views;

    auto print_category{
      [](std::string_view extension, const std::vector<fs::path>& paths)
      {
        const auto paths_size{ paths.size() };

        fmt::println("{} ({} {}):",
                     extension,
                     paths_size,
                     (paths_size > 1 ? "files" : "file"));

        for ( const auto& path : paths )
        {
          fmt::println("{:>4} {}", '-', path.filename().string());
        }

        fmt::println("");
      }
    };

    fmt::println("Files organized by extension:\n");

    if ( auto iterator{ files.find(g_NO_EXTENSION) }; iterator != files.end() )
      print_category(iterator->first, iterator->second);

    for ( const auto& [extension, paths] :
          files | vws::filter([](const auto& pair)
                              { return pair.first != g_NO_EXTENSION; }) )
    {
      print_category(extension, paths);
    }
  }
} // namespace

auto main() -> int
{
  fs::path target_dir{
    "./resources/test_files"
  }; // or pass via command line argument later

  if ( not is_valid_directory(target_dir) )
  {
    fmt::println(
          stderr, "Error: '{}' is not a valid directory", target_dir.string());
    return 1;
  }

  auto files{ collect_files_by_extension(target_dir) };

  if ( !files )
  {
    fmt::println("{}", files.error().message());
    return 0;
  }

  if ( files->empty() )
  {
    fmt::println("No files found in directory.");
    return 0;
  }

  display_results(*files);

  return 0;
}

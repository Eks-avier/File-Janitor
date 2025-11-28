#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <flat_map>
#include <flat_set>
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <functional>
#include <optional>
#include <ranges>
#include <scn/scan.h>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace
{
  namespace fs = std::filesystem;

  template <typename... Types>
  struct Overloaded : Types...
  {
    using Types::operator()...;
  };

  using FilesByExtension
        = std::flat_map<std::string, std::vector<fs::path>, std::less<>>;

  using ExtensionList
        = std::variant<std::vector<std::string_view>, std::vector<std::string>>;

  using FolderGroups = std::flat_map<std::string, ExtensionList, std::less<>>;

  namespace constants // g_ for global constants
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

  enum class FileOrganizerError : std::uint8_t
  {
    directory_iterator_failed,
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

  enum class FolderCategory : std::uint8_t
  {
    regular,
    others,
    no_extension
  };

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

  struct Folder
  {
    std::string           base_name;
    std::optional<int>    collision_suffix;
    FolderCategory        category;
    ExtensionList         extensions;
    std::vector<fs::path> files;
  };

  [[maybe_unused]] auto get_resolved_name(const Folder& folder) -> std::string
  {
    return folder.collision_suffix
                 ? fmt::format(
                         "{} ({})", folder.base_name, *folder.collision_suffix)
                 : folder.base_name;
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

  [[maybe_unused]] auto is_valid_directory(const fs::path& directory) -> bool
  {
    return fs::exists(directory) and fs::is_directory(directory);
  }

  [[maybe_unused]] auto collect_files_by_extension(const fs::path& dir)
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

  [[nodiscard]] auto
  get_existing_folders(const fs::path& target_directory) noexcept
        -> std::expected<std::vector<std::string>, DirectoryScanError>
  {
    assert(is_valid_directory(target_directory));

    using enum FileOrganizerError;

    auto container{ std::vector<std::string>{} };
    auto code{ std::error_code{} };

    for ( auto iterator{ fs::directory_iterator(target_directory, code) };
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

      if ( not entry.is_directory() )
      {
        continue;
      }

      container.emplace_back(entry.path().filename().string());
    }

    if ( code )
    {
      return std::unexpected{
        DirectoryScanError{ directory_iterator_failed, code }
      };
    }

    return container;
  }

  [[nodiscard]] auto get_collision_suffix(
        std::string_view                base_name,
        const std::vector<std::string>& existing_folder_names) noexcept
        -> std::optional<int>
  {
    if ( not std::ranges::contains(existing_folder_names, base_name) )
    {
      return {};
    }

    auto suffixes{ std::vector<int>{} };

    for ( auto        fmt_string{ fmt::format("{} ({})", base_name, "{}") };
          const auto& folder_name : existing_folder_names )
    {
      if ( auto result{ scn::scan<int>(
                 folder_name, scn::runtime_format(fmt_string)) } )
      {
        suffixes.push_back(result->value());
      }
    }

    return suffixes.empty() ? 1 : std::ranges::max(suffixes) + 1;
  }

  [[maybe_unused]] auto create_folder_groups(const FilesByExtension& files)
        -> FolderGroups
  {
    assert(not files.empty());

    namespace vws = std::views;
    namespace rng = std::ranges;

    auto process_chunk{
      [](auto chunk) -> std::pair<std::string, ExtensionList>
      {
        auto folder{ std::string_view{ chunk.front().first } };
        auto extensions{ chunk | vws::values };

        auto to_static_view{ [](auto&& extension) -> auto
                             { return *get_known_extension(extension); } };

        auto variant{
          folder == constants::g_OTHERS_FOLDER_NAME
                ? ExtensionList{ extensions
                                 | rng::to<std::vector<std::string>>() }
                : ExtensionList{ extensions
                                 | vws::transform(to_static_view)
                                 | rng::to<std::vector<std::string_view>>() }
        };

        return { std::string{ folder }, std::move(variant) };
      }
    };

    auto grouped_pairs{
      files
      | vws::keys
      | vws::transform(
            [](const auto& extension) -> auto
            { return std::pair{ get_folder_name(extension), extension }; })
      | rng::to<std::vector<std::pair<std::string_view, std::string>>>()
    };

    rng::sort(grouped_pairs); // Sorts folder names and exts lexicographically

    return grouped_pairs
           | vws::chunk_by([](const auto& lhs, const auto& rhs) -> auto
                           { return lhs.first == rhs.first; })
           | vws::transform(process_chunk)
           | rng::to<FolderGroups>();
  }

  auto collect_folder_files(const ExtensionList&    extensions,
                            const FilesByExtension& data_source)
        -> std::vector<fs::path>
  {
    namespace vws = std::views;
    namespace rng = std::ranges;

    auto lookup_path{
      [&](std::string_view extension) -> std::span<const fs::path>
      {
        if ( auto it{ data_source.find(extension) }; it != data_source.end() )
        {
          return it->second;
        }
        return {};
      }
    };

    auto paths{ std::visit(
          [&](const auto& extension_container) -> auto
          {
            return extension_container
                   | vws::transform(lookup_path)
                   | vws::join
                   | rng::to<std::vector<fs::path>>();
          },
          extensions) };

    rng::sort(paths);
    return paths;
  }

  auto create_folders(FolderGroups&&                  folder_groups,
                      const std::vector<std::string>& existing_folders,
                      const FilesByExtension&         files_source)
        -> std::vector<Folder>
  {
    assert(not folder_groups.empty());

    namespace vws = std::views;
    namespace rng = std::ranges;

    auto to_folder{
      [&](auto&& group) -> Folder
      {
        auto  base_name{ std::string{ std::move(group.first) } };
        auto& extensions{ group.second };

        auto suffix{ get_collision_suffix(base_name, existing_folders) };
        auto category{ get_folder_category(base_name) };

        auto files{ collect_folder_files(extensions, files_source) };

        return { .base_name{ std::move(base_name) },
                 .collision_suffix{ suffix },
                 .category{ category },
                 .extensions{ extensions },
                 .files{ std::move(files) } };
      }
    };

    return std::move(folder_groups)
           | vws::transform(to_folder)
           | rng::to<std::vector<Folder>>();
  }

  auto create_organization_plan(const FilesByExtension& files,
                                const fs::path&         target_directory)
        -> std::expected<std::vector<Folder>, DirectoryScanError>
  {
    assert(is_valid_directory(target_directory));

    return get_existing_folders(target_directory)
          .transform(
                [&](const auto& existing) -> auto
                {
                  return create_folders(
                        create_folder_groups(files), existing, files);
                });
  }

  [[maybe_unused]] auto
  print_colliding_folders(const std::vector<Folder>& folders) -> void
  {
    assert(not folders.empty());

    using namespace std::string_view_literals;

    constexpr auto header_message{ "⚠️ COLLISION WARNINGS"sv };
    constexpr auto header_format{ "{} ({} detected):\n"sv };

    const auto styled_header{ fmt::styled(
          header_message, fg(fmt::color::red) | fmt::emphasis::bold) };

    fmt::print(header_format, styled_header, folders.size());

    auto by_collision{ [](const Folder& folder) -> bool
                       { return folder.collision_suffix.has_value(); } };

    constexpr auto list_format{ "{:>4} {} -> {}\n"sv };
    constexpr auto list_marker{ "•"sv };

    const auto list_style{ fg(fmt::color::red) };

    for ( const auto& folder : folders | std::views::filter(by_collision) )
    {
      fmt::print(list_style,
                 list_format,
                 list_marker,
                 folder.base_name,
                 get_resolved_name(folder));
    }
  }

  auto display_organization_plan(const std::vector<Folder>& folders) -> void
  {
    assert(not folders.empty());

    using namespace std::string_view_literals;
    using namespace constants;

    constexpr auto separator{ "═"sv };
    constexpr auto separator_size{ 50 };

    auto print_separator{
      [separator, separator_size] -> void
      { fmt::println("\n{:═^{}}\n", separator, separator_size); }
    };

    fmt::println("\n{:^{}}", "Organization Plan", separator_size);
    print_separator();

    // 1. COLLISION WARNING
    print_colliding_folders(folders);

    print_separator();

    constexpr auto notification_format{ "Will contain {} {}\n"sv };

    const auto print_notification{
      [notification_format](const std::size_t file_count) -> void
      {
        const auto message_continuation_style{ fmt::styled(
              "files", fg(fmt::color::dim_gray)) };
        const auto count_style{ fmt::styled(
              file_count, fg(fmt::color::white) | fmt::emphasis::bold) };

        fmt::print((fg(fmt::color::dim_gray)),
                   notification_format,
                   count_style,
                   message_continuation_style);
      }
    };

    constexpr auto file_list_marker{ '-' };
    constexpr auto file_list_padding{ 4 };

    static const auto file_list_format{ fmt::format(
          "{0:>{1}} {{}}", file_list_marker, file_list_padding) };

    auto print_folder_header{
      [](const Folder& folder) -> void
      {
        fmt::print("\n{}/\n",
                   fmt::styled(get_resolved_name(folder),
                               get_folder_style(folder.category)));
      }
    };

    auto print_folder_files{ [](const std::vector<fs::path>& paths) -> void
                             {
                               for ( const auto& path : paths )
                               {
                                 fmt::println(fmt::runtime(file_list_format),
                                              path.filename().string());
                               }
                             } };

    auto print_folders{
      [print_notification, print_folder_files, print_folder_header](
            const std::vector<Folder>& raw_folders,
            FolderCategory             category) -> void
      {
        auto by_category{ std::views::filter(
              [category](const Folder& folder) -> bool
              { return folder.category == category; }) };

        for ( const auto& folder : raw_folders | by_category )
        {
          print_folder_header(folder);
          print_notification(folder.files.size());
          print_folder_files(folder.files);
        }
      }
    };

    // 2. REGULAR FOLDERS
    print_folders(folders, FolderCategory::regular);

    // 3. OTHERS FOLDERS
    print_folders(folders, FolderCategory::others);

    // 4. NO EXTENSION FOLDERS
    print_folders(folders, FolderCategory::no_extension);

    // 5. FOOTER
    const auto total_folders{ folders.size() };
    const auto total_files = std::ranges::fold_left(
          folders
                | std::views::transform(&Folder::files)
                | std::views::transform(&std::vector<fs::path>::size),
          0UZ,
          std::plus<>{});

    print_separator();
    fmt::print(fg(fmt::color::green) | fmt::emphasis::bold,
               "Total: {} folders, {} files\n\n",
               total_folders,
               total_files);
  }

  [[maybe_unused]] auto display_results(const FilesByExtension& files) noexcept
        -> void
  {
    using namespace constants;

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
    {
      print_category(iterator->first, iterator->second);
    }

    for ( const auto& [extension, paths] :
          files
                | std::views::filter([](const auto& pair) -> auto
                                     { return pair.first != g_NO_EXTENSION; }) )
    {
      print_category(extension, paths);
    }
  }

} // namespace

auto main() -> int
{
  fs::path target_dir = "./resources/test_files";

  // Validate directory
  if ( !is_valid_directory(target_dir) )
  {
    fmt::println(
          stderr, "Error: '{}' is not a valid directory", target_dir.string());
    return 1;
  }

  // Collect files
  auto files_result = collect_files_by_extension(target_dir);
  if ( !files_result )
  {
    fmt::println(stderr, "Error: {}", files_result.error().message());
    return 1;
  }

  // Create organization plan (Sprint 2!)
  auto plan_result = create_organization_plan(*files_result, target_dir);
  if ( !plan_result )
  {
    fmt::println(stderr, "Error: {}", plan_result.error().message());
    return 1;
  }

  // Display the plan (dry-run preview)
  display_organization_plan(*plan_result);

  return 0;
}

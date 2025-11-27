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
#include <fmt/compile.h>
#include <fmt/format.h>
#include <functional>
#include <iterator>
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
          [extension](const auto& pair) { return pair.first == extension; }) };

    return iterator != g_KNOWN_EXTENSIONS.end() ? iterator->second
                                                : g_OTHERS_FOLDER_NAME;
  }

  constexpr auto get_known_extension(std::string_view requested_extension)
        -> std::optional<std::string_view>
  {
    using namespace constants;

    const auto* iterator{ std::ranges::find_if(
          g_KNOWN_EXTENSIONS,
          [requested_extension](const auto& pair)
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

  using ExtensionList
        = std::variant<std::vector<std::string_view>, std::vector<std::string>>;

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

  [[nodiscard]] auto find_collision_suffix(
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

  auto collect_unique_folders(const FilesByExtension& files)
        -> std::flat_set<std::string>
  {
    assert(not files.empty());

    auto container{ std::flat_set<std::string>{} };

    for ( const auto& [extension, _] : files ) // NOLINT
    {
      container.insert(get_folder_name(extension));
    }

    return container;
  }

  auto initialize_folder_groups(const std::flat_set<std::string>& folder_names)
        -> std::flat_map<std::string, ExtensionList, std::less<>>
  {
    assert(not folder_names.empty());

    return folder_names
           | std::views::transform(
                 [](const auto& folder_name) -> auto
                 {
                   return std::pair{
                     folder_name,
                     folder_name == constants::g_OTHERS_FOLDER_NAME
                           ? ExtensionList{ std::vector<std::string>{} }
                           : ExtensionList{ std::vector<std::string_view>{} }
                   };
                 })
           | std::ranges::to<
                 std::flat_map<std::string, ExtensionList, std::less<>>>();
  }

  auto populate_folder_extensions(
        const FilesByExtension&                                 collected_files,
        std::flat_map<std::string, ExtensionList, std::less<>>& folder_groups)
        -> void
  {
    assert(not collected_files.empty() and not folder_groups.empty());

    for ( const auto& [extension, _] : collected_files )
    {
      auto folder_name{ get_folder_name(extension) };

      std::visit(
            Overloaded{
                  [&extension](std::vector<std::string_view>& viewers) -> void
                  {
                    if ( auto extension_view{ get_known_extension(extension) } )
                    {
                      viewers.push_back(*extension_view);
                    }
                  },
                  [&extension](std::vector<std::string>& owners) -> void
                  { owners.push_back(extension); } },
            folder_groups[std::string{ folder_name }]);
    }
  }

  using FolderGroups = std::flat_map<std::string, ExtensionList, std::less<>>;

  auto create_folders_with_collision_detection(
        FolderGroups&&                  folder_groups,
        const std::vector<std::string>& existing_folders,
        const FilesByExtension& files_by_extension) -> std::vector<Folder>
  {
    assert(not folder_groups.empty());

    auto folders{ std::vector<Folder>{} };

    for ( auto&& folder_group : std::move(folder_groups) )
    {
      auto  folder_name{ std::string_view{ folder_group.first } };
      auto& folder_extensions{ folder_group.second };

      auto folder_suffix{ find_collision_suffix(
            folder_name, existing_folders) };

      auto get_files{
        [](const FilesByExtension& files,
           std::string_view extension) -> std::optional<std::vector<fs::path>>
        {
          return files.contains(extension)
                       ? std::make_optional(files.at(extension))
                       : std::nullopt;
        }
      };

      auto folder_files{ std::vector<fs::path>{} };

      std::visit(
            Overloaded{
                  [&files_by_extension, &folder_files, get_files](
                        const std::vector<std::string_view>& viewing_extensions)
                        -> void
                  {
                    for ( const auto extension : viewing_extensions )
                    {
                      auto files{ get_files(files_by_extension, extension) };
                      folder_files.append_range(
                            files ? *files : std::vector<fs::path>{});
                    }
                  },
                  [&files_by_extension, &folder_files, get_files](
                        const std::vector<std::string>& owning_extensions)
                        -> void
                  {
                    for ( const auto& extension : owning_extensions )
                    {
                      auto files{ get_files(files_by_extension, extension) };
                      folder_files.append_range(
                            files ? *files : std::vector<fs::path>{});
                    }
                  } },
            folder_extensions);

      std::ranges::sort(folder_files);

      auto folder_category{ get_folder_category(folder_name) };

      folders.emplace_back(Folder{ .base_name{ folder_name },
                                   .collision_suffix{ folder_suffix },
                                   .category{ folder_category },
                                   .extensions{ std::move(folder_extensions) },
                                   .files{ folder_files } });
    }

    std::ranges::sort(folders, {}, &Folder::base_name);
    return folders;
  }

  auto create_organization_plan(const FilesByExtension& files,
                                const fs::path&         target_directory)
        -> std::expected<std::vector<Folder>, DirectoryScanError>
  {
    assert(is_valid_directory(target_directory));

    auto existing_folders{ get_existing_folders(target_directory) };

    if ( not existing_folders )
    {
      return std::unexpected{ existing_folders.error() };
    }

    auto unique_folder_names{ collect_unique_folders(files) };

    auto folder_groups{ initialize_folder_groups(unique_folder_names) };

    populate_folder_extensions(files, folder_groups);

    return create_folders_with_collision_detection(
          std::move(folder_groups), *existing_folders, files);
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
    auto colliding_folders{
      folders
      | std::views::filter([](const Folder& folder) -> bool
                           { return folder.collision_suffix.has_value(); })
      | std::ranges::to<std::vector<Folder>>()
    };

    constexpr auto warning_header{ "⚠️ COLLISION WARNINGS"sv };

    fmt::print("{} ({} detected):\n",
               fmt::styled(warning_header,
                           fg(fmt::color::red) | fmt::emphasis::bold),
               colliding_folders.size());

    // PRINT COLLIDED FOLDERS
    if ( not std::ranges::empty(colliding_folders) )
    {
      for ( const auto& folder : colliding_folders )
      {
        fmt::print("{:>4} {} -> {}\n",
                   "•",
                   fmt::styled(folder.base_name, fg(fmt::color::red)),
                   fmt::styled(get_resolved_name(folder),
                               fg(fmt::color::red) | fmt::emphasis::bold));
      }
    }

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

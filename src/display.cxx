#include <file_janitor/display.hxx>

#include <cassert>
#include <fmt/color.h>
#include <fmt/format.h>
#include <functional>
#include <ranges>
#include <string_view>

namespace file_janitor
{
  namespace fs = std::filesystem;

  FolderDisplayContext::FolderDisplayContext(std::span<const Folder> folders)
      : folders_(folders)
  {}

  auto FolderDisplayContext::print_title() -> FolderDisplayContext&
  {
    fmt::println("\n{:^{}}", "Organization Plan", separator_width_);
    print_separator();
    return *this;
  }

  auto FolderDisplayContext::print_collisions() -> FolderDisplayContext&
  {
    using namespace std::string_view_literals;

    auto has_collided = [](const Folder& f)
    { return f.collision_suffix.has_value(); };

    auto       collision_view{ folders_ | std::views::filter(has_collided) };
    const auto count{ std::ranges::distance(collision_view) };

    if ( count == 0 )
    {
      return *this;
    }

    constexpr auto header_msg{ "⚠️ COLLISION WARNINGS"sv };
    constexpr auto header_fmt{ "{} ({} detected):\n"sv };

    fmt::print(
          header_fmt,
          fmt::styled(header_msg, fg(fmt::color::red) | fmt::emphasis::bold),
          count);

    constexpr auto item_fmt{ "{:>4} {} -> {}\n"sv };
    const auto     item_style = fg(fmt::color::red);

    for ( const auto& folder : collision_view )
    {
      fmt::print(item_style,
                 item_fmt,
                 "•",
                 folder.base_name,
                 get_resolved_name(folder));
    }

    print_separator();
    return *this;
  }

  auto FolderDisplayContext::print_category(FolderCategory category) -> FolderDisplayContext&
  {
    auto by_category = [category](const Folder& f)
    { return f.category == category; };

    for ( const auto& folder : folders_ | std::views::filter(by_category) )
    {
      print_single_folder(folder);
    }
    return *this;
  }

  auto FolderDisplayContext::print_summary() -> void
  {
    using namespace std::views;

    const auto total_folders = folders_.size();

    const auto total_files = std::ranges::fold_left(
          folders_
                | transform(&Folder::files)
                | transform(&std::vector<fs::path>::size),
          0UZ,
          std::plus<>{});

    print_separator();
    fmt::print(fg(fmt::color::green) | fmt::emphasis::bold,
               "Total: {} folders, {} files\n\n",
               total_folders,
               total_files);
  }

  void FolderDisplayContext::print_separator()
  {
    fmt::println("\n{:═^{}}\n", separator_char_, separator_width_);
  }

  void FolderDisplayContext::print_single_folder(const Folder& folder)
  {
    fmt::print("\n{}/\n",
               fmt::styled(get_resolved_name(folder),
                           get_folder_style(folder.category)));

    fmt::print(fg(fmt::color::dim_gray),
               "Will contain {} {}\n",
               fmt::styled(folder.files.size(),
                           fg(fmt::color::white) | fmt::emphasis::bold),
               fmt::styled("files", fg(fmt::color::dim_gray)));

    constexpr auto file_indent = 4;
    for ( const auto& path : folder.files )
    {
      fmt::println(
            "{0:>{1}} {2}", "-", file_indent, path.filename().string());
    }
  }

  auto display_organization_plan(const std::vector<Folder>& folders) -> void
  {
    assert(not folders.empty());

    using enum FolderCategory;

    FolderDisplayContext{ folders }
          .print_title()
          .print_collisions()
          .print_category(regular)
          .print_category(others)
          .print_category(no_extension)
          .print_summary();
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

    for ( auto&& [extension, paths] :
          files
                | std::views::filter([](const auto& pair) -> auto
                                     { return pair.first != g_NO_EXTENSION; }) )
    {
      print_category(extension, paths);
    }
  }

} // namespace file_janitor

#include <algorithm>
#include <cassert>
#include <ctre.hpp>
#include <file_janitor/organization.hxx>
#include <fmt/format.h>
#include <functional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

namespace file_janitor
{
  namespace vws = std::views;
  namespace rng = std::ranges;
  namespace fs  = std::filesystem;

  constexpr auto parse_int(std::string_view view) noexcept -> std::optional<int>
  {
    auto value{ 0 };

    const auto result{ std::from_chars(
          view.data(), view.data() + view.length(), value) }; // NOLINT

    if ( (result.ec == std::errc{})
         and (result.ptr == (view.data() + view.length())) ) // NOLINT
    {
      return value;
    }

    return {};
  }

  [[nodiscard]] auto get_collision_suffix(
        std::string_view                base_name,
        const std::vector<std::string>& existing_folder_names) noexcept
        -> std::optional<int>
  {
    namespace rng = std::ranges;
    namespace vws = std::views;

    if ( not rng::contains(existing_folder_names, base_name) )
    {
      return {};
    }

    static constexpr auto suffix_pattern{ ctll::fixed_string{
          R"(^ \((\d+)\)$)" } };

    const auto suffixes{
      existing_folder_names
      | vws::filter([base_name](std::string_view folder) -> bool
                    { return folder.starts_with(base_name); })
      | vws::transform([base_name](std::string_view folder) -> std::string_view
                       { return folder.substr(base_name.length()); })
      | vws::transform(
            [](std::string_view suffix) -> std::optional<int>
            {
              if ( auto match{ ctre::match<suffix_pattern>(suffix) } )
              {
                return parse_int(match.get<1>());
              }
              return {};
            })
      | vws::filter([](const auto& opt) -> bool { return opt.has_value(); })
      | vws::transform([](const auto& opt) -> auto { return *opt; })
      | rng::to<std::vector>()
    };

    return suffixes.empty() ? 1 : rng::max(suffixes) + 1;
  }

  [[maybe_unused]] auto create_folder_groups(const FilesByExtension& files)
        -> FolderGroups
  {
    assert(not files.empty());

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

    rng::sort(grouped_pairs);

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

} // namespace file_janitor

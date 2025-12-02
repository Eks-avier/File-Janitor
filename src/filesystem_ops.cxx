#include <file_janitor/filesystem_ops.hxx>

#include <cassert>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

namespace file_janitor
{
  namespace fs = std::filesystem;

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

} // namespace file_janitor

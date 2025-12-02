#ifndef FILE_JANITOR_ERRORS_HXX
#define FILE_JANITOR_ERRORS_HXX

#include <cstdint>
#include <fmt/format.h>
#include <string>
#include <string_view>
#include <system_error>

namespace file_janitor
{
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

} // namespace file_janitor

#endif // FILE_JANITOR_ERRORS_HXX

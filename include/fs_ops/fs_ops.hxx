//
// Created by Xavier on 12/16/2025.
//

#ifndef FILEJANITOR_FS_OPS_HXX
#define FILEJANITOR_FS_OPS_HXX

#include <filesystem>

// TODO: Add value-types with fluent interface, wherever possible

namespace fs_ops
{
  struct scanned_file
  {
    std::filesystem::path path;
    std::string           extension;
  };

  struct candidate
  {
    std::filesystem::path parent;
    std::string           stem;
    std::string           extension;
  };

  enum class operation_status : std::int8_t
  {
    failure,
    success,
    skipped,
  };

  struct successful_operation
  {
    std::filesystem::path source;
    std::filesystem::path destination;
    std::string           bucket_name;
  };

  struct failed_operation
  {
    std::filesystem::path source;
    std::filesystem::path destination;
    std::error_code       error;
  };

} // namespace fs_ops

#endif // FILEJANITOR_FS_OPS_HXX

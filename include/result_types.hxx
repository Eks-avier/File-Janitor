//
// Created by Xavier on 12/16/2025.
//

#ifndef FILEJANITOR_RESULT_TYPES_HXX
#define FILEJANITOR_RESULT_TYPES_HXX

#include <expected>
#include <filesystem>
#include <system_error>
#include <vector>

template <typename T>
using Result = std::expected<T, std::error_code>;

using VoidResult = Result<void>;
using ScanResult = Result<std::filesystem::directory_entry>;

#endif // FILEJANITOR_RESULT_TYPES_HXX

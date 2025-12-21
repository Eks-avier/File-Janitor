//
// Created by Xavier on 12/16/2025.
//

#include "safe_fs.hxx"
#include "result_types.hxx"
#include <filesystem>
#include <generator>
#include <system_error>
#include <utility>

namespace fs = std::filesystem;

namespace safe_fs
{
  auto safe_scan(const fs::path path) -> std::generator<ScanResult>
  {
    auto ec{ std::error_code{} };

    auto dir_it{
      fs::directory_iterator{ path, ec }
    };

    if ( ec )
    {
      co_yield std::unexpected{ ec };
      co_return;
    }

    for ( const auto end_it{ fs::directory_iterator{} }; dir_it != end_it; dir_it.increment(ec) )
    {
      if ( ec )
      {
        co_yield std::unexpected{ ec };
        co_return;
      }

      co_yield *dir_it;
    }
  }

  auto exists(const fs::path& path) noexcept -> bool
  {
    return [&path, ec{ std::error_code{} }] mutable -> bool {
      return fs::exists(path, ec);
    }();
  }

  auto rename(const fs::path& from, const fs::path& to) -> VoidResult
  {
    return [&from, &to, ec{ std::error_code{} }] mutable -> VoidResult {
      fs::rename(from, to, ec);
      return ec ? std::unexpected{ ec } : VoidResult{};
    }();
  }

  auto create_directories(const fs::path& path) -> VoidResult
  {
    // create_directories returns false if dir already exists, but we treat that as success.
    // We only fail if ec is set.
    return [&path, ec{ std::error_code{} }] mutable -> VoidResult {
      fs::create_directories(path, ec);
      return ec ? std::unexpected{ ec } : VoidResult{};
    }();
  }

} // namespace safe_fs

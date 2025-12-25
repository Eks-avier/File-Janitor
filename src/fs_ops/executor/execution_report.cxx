//
// Created by Xavier on 12/25/2025.
//

module;

// Standard headers in GMF
#include <cstdint>
#include <span>
#include <vector>

module filejanitor;

// NO import std; - use GMF includes for consistency

namespace fs_ops::executor
{
  auto execution_report::success_count() const noexcept -> int
  {
    return success_count_;
  }

  auto execution_report::processed_count() const noexcept -> int
  {
    return processed_count_;
  }

  auto execution_report::failure_count() const noexcept -> std::int64_t
  {
    return std::ssize(failures_);
  }

  auto execution_report::skipped_count() const noexcept -> std::int64_t
  {
    return processed_count_ - success_count_ - failure_count();
  }

  auto execution_report::failures() const noexcept
              -> std::span<const fs_ops::failed_operation>
  {
    return failures_;
  }
} // namespace fs_ops::executor

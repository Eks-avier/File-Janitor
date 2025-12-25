//
// Created by Xavier on 12/25/2025.
//

import std;

#include "operation_result.hxx"

namespace fs_ops
{
  operation_result::operation_result(const operation_status status)
      : status_{status}
  {}

  auto operation_result::create_success() -> operation_result
  {
    return operation_result{operation_status::success};
  }

  auto operation_result::create_failure(
              const successful_operation& o,
              const std::error_code       ec
  ) -> operation_result
  {
    return operation_result{operation_status::failure}.with_failure(o, ec);
  }

  auto operation_result::create_skipped() -> operation_result
  {
    return operation_result{operation_status::skipped};
  }
  auto operation_result::status() const -> operation_status { return status_; }

  auto operation_result::failure() const
              -> std::optional<std::reference_wrapper<const failed_operation>>
  {
    return status_ != operation_status::failure ? std::nullopt
                                                : std::optional{std::cref(failure_)};
  }

} // namespace fs_ops

//
// Created by Xavier on 12/25/2025.
//

#ifndef FILEJANITOR_OPERATION_RESULT_HXX
#define FILEJANITOR_OPERATION_RESULT_HXX

#include "fs_ops.hxx"
#include <functional>
#include <optional>
#include <system_error>
#include <utility>

namespace fs_ops
{
  class operation_result
  {
  public:
    operation_result() = delete;

    static auto create_success() -> operation_result;

    static auto
    create_failure(const successful_operation& o, const std::error_code ec)
                -> operation_result;

    static auto create_skipped() -> operation_result;

    [[nodiscard]] auto status() const -> operation_status;

    [[nodiscard]] auto failure() const
                -> std::optional<std::reference_wrapper<const failed_operation>>;

  private:
    failed_operation failure_{};
    operation_status status_{};

    explicit operation_result(const operation_status status);

    template <typename Self>
    auto with_failure(
                this Self&&                 self,
                const successful_operation& op,
                const std::error_code       ec
    ) -> operation_result
    {
      self.failure_ = failed_operation{
           .source{op.source},
           .destination{op.destination},
           .error{ec}
      };
      return std::forward<Self>(self);
    }
  };

} // namespace fs_ops

#endif // FILEJANITOR_OPERATION_RESULT_HXX

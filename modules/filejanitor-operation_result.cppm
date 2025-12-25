// Module partition: operation_result
// Exports: operation_result class
// Depends on: fs_ops
module;

#include <filesystem>
#include <functional>
#include <optional>
#include <system_error>
#include <utility>

export module filejanitor:operation_result;

// Re-export dependency partition
export import :fs_ops;

// Define and export operation_result class
export namespace fs_ops {
    class operation_result {
    public:
        operation_result() = delete;

        static auto create_success() -> operation_result;
        static auto create_failure(const successful_operation& o, std::error_code ec) -> operation_result;
        static auto create_skipped() -> operation_result;

        [[nodiscard]] auto status() const -> operation_status;
        [[nodiscard]] auto failure() const -> std::optional<std::reference_wrapper<const failed_operation>>;

    private:
        failed_operation failure_{};
        operation_status status_{};

        explicit operation_result(operation_status status);

        template <typename Self>
        auto with_failure(this Self&& self, const successful_operation& op, std::error_code ec) -> operation_result {
            self.failure_ = failed_operation{
                .source{op.source},
                .destination{op.destination},
                .error{ec}
            };
            return std::forward<Self>(self);
        }
    };
}

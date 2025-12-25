// Module partition: execution_report
// Exports: execution_report class
// Depends on: fs_ops
module;

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

export module filejanitor:execution_report;

// Re-export dependency partition
export import :fs_ops;

// Define and export execution_report class
export namespace fs_ops::executor {
    class execution_report {
    public:
        static auto start() noexcept -> execution_report { return {}; }

        [[nodiscard]] auto success_count() const noexcept -> int;
        [[nodiscard]] auto processed_count() const noexcept -> int;
        [[nodiscard]] auto failure_count() const noexcept -> std::int64_t;
        [[nodiscard]] auto skipped_count() const noexcept -> std::int64_t;
        [[nodiscard]] auto failures() const noexcept -> std::span<const failed_operation>;

        template <typename Self>
        auto with_processed(this Self&& self) noexcept -> Self&& {
            ++self.processed_count_;
            return std::forward<Self>(self);
        }

        template <typename Self>
        auto with_success(this Self&& self) noexcept -> Self&& {
            ++self.success_count_;
            return std::forward<Self>(self);
        }

        template <typename Self>
        auto with_failure(this Self&& self, const failed_operation& failure) noexcept -> Self&& {
            self.failures_.push_back(failure);
            return std::forward<Self>(self);
        }

        template <typename Self>
        auto finalize(this Self&& self) noexcept -> execution_report {
            return std::forward<Self>(self);
        }

    private:
        execution_report() = default;

        std::vector<failed_operation> failures_{};
        int processed_count_{0};
        int success_count_{0};
    };
}

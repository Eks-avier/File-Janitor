// Module partition: fs_ops
// Exports: scanned_file, candidate, operation_status, successful_operation, failed_operation
module;

#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>

export module filejanitor:fs_ops;

// Define and export types directly in the module
export namespace fs_ops {
    struct scanned_file {
        std::filesystem::path path;
        std::string           extension;
    };

    struct candidate {
        std::filesystem::path parent;
        std::string           stem;
        std::string           extension;
    };

    enum class operation_status : std::int8_t {
        failure,
        success,
        skipped,
    };

    struct successful_operation {
        std::filesystem::path source;
        std::filesystem::path destination;
        std::string           bucket_name;
    };

    struct failed_operation {
        std::filesystem::path source;
        std::filesystem::path destination;
        std::error_code       error;
    };
}

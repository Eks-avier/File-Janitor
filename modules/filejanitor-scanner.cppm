// Module partition: scanner
// Exports: file_collection, collect_files()
module;

#include <filesystem>
#include <system_error>
#include <vector>

// Include header in global module fragment
#include "fs_ops/scanner/scanner.hxx"

export module filejanitor:scanner;

// Re-export types and functions from fs_ops::scanner namespace
export namespace fs_ops::scanner {
    using ::fs_ops::scanner::file_collection;
    using ::fs_ops::scanner::collect_files;
}

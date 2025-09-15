#ifndef _LABWORK8_LIB_DIR_HPP_
#define _LABWORK8_LIB_DIR_HPP_

#include <filesystem>
#include <vector>

#include "pipeline.hpp"
#include "struct.hpp"

// Collects all files in a directory (optionally recursively through
// all subdirectories).
template <template <typename, typename...> typename Container = std::vector,
          typename... Args>
inline auto Dir(std::filesystem::path dir_path, bool is_recursive) {
    if (!std::filesystem::is_directory(dir_path)) {
        throw std::runtime_error("Input path is not a directory");
    }

    // Uses Pipeline as a return type
    using PipelineFlow =
        Pipeline<std::filesystem::path, Container, Args...>;

    return PipelineFlow(
        [is_recursive, dir_path](typename PipelineFlow::Container& output) {
            if (is_recursive) {
                output = typename PipelineFlow::Container(
                    std::filesystem::recursive_directory_iterator(dir_path),
                    std::filesystem::recursive_directory_iterator());
            } else {
                output = typename PipelineFlow::Container(
                    std::filesystem::directory_iterator(dir_path),
                    std::filesystem::directory_iterator());
            }
        });
}

#endif  // _LABWORK8_LIB_DIR_HPP_

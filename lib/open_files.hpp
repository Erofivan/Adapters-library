#ifndef _LABWORK8_LIB_OPEN_FILES_HPP
#define _LABWORK8_LIB_OPEN_FILES_HPP

#include <filesystem>
#include <functional>
#include <fstream>

#include "struct.hpp"
#include "transform.hpp"

template <
    template <typename, typename...> typename Container,
    typename... Args>
class OpenFilesAdapter
    : public TransformAdapter<
          std::function<std::ifstream(std::filesystem::path)>,
          Container,
          Args...> {
public:
    // Opens an input file stream for given path
    OpenFilesAdapter()
        : TransformAdapter<
              std::function<std::ifstream(std::filesystem::path)>,
              Container,
              Args...>(
              [](std::filesystem::path path) {
                  return std::ifstream(path);
              }) {}
};

// Opens an input file stream for each path received from the previous adapter.
template <
    template <typename, typename...> typename Container = SameContainer,
    typename... Args>
inline auto OpenFiles() {
    return OpenFilesAdapter<Container, Args...>();
}

#endif  // _LABWORK8_LIB_OPEN_FILES_HPP

#ifndef _LABWORK8_LIB_SPLIT_HPP_
#define _LABWORK8_LIB_SPLIT_HPP_

#include <memory>
#include <string>

#include "pipeline.hpp"
#include "struct.hpp"

template <
    template <typename, typename...> typename Container,
    typename... Args>
class SplitAdapter {
private:
    // Stores delimiter characters and a cached lookup table for fast splitting
    // This structure works faster than regular std::unordered map or std::map
    // But works only with ASCII characters
    struct DelimiterMap {
        static const size_t kMapSize = 256;
        static const size_t kBufferSize = 1024;

        std::string delimiters;
        bool map[kMapSize];
        bool is_cached = false;

        DelimiterMap(std::string delimiters)
            : delimiters(std::move(delimiters)) {}

        // Fills lookup table so that map[c] is true iff c is a delimiter
        void Cache() {
            if (is_cached) return;
            is_cached = true;

            for (size_t i = 0; i < kMapSize; ++i) {
                map[i] = false;
            }

            for (char c : delimiters) {
                map[static_cast<int>(c)] = true;
            }
        }
    };

    std::shared_ptr<DelimiterMap> delim_map_;

public:
    virtual ~SplitAdapter() = default;
    
    // Constructs a splitting adapter with the given delimiter characters
    explicit SplitAdapter(const std::string& delimiters)
        : delim_map_(std::make_shared<DelimiterMap>(delimiters)) {}

    // Applies split operation on a container of std::strings 
    // (e.g., std::vector<std::string>)
    // Each string is split into substrings using the configured delimiters
    template <typename DataFlow>
        requires std::convertible_to<typename DataFlow::Type, std::string>
    auto apply(DataFlow& flow) const {
        auto local_map = delim_map_;
        return flow.template derive<std::string, Container, Args...>(
            [local_map](auto& input, auto& output) {
                local_map->Cache();

                for (std::string& str : input) {
                    int left = 0;
                    for (int right = 0; right < str.size(); ++right) {
                        if (local_map->map[str[right]]) {
                            output.push_back(str.substr(left, right - left));
                            left = right + 1;
                        }
                    }
                    output.push_back(str.substr(left));
                }
            });
    }

    // Applies split operation to a container of std::istream 
    // (e.g., std::vector<std::stringstream>)
    // Streams are read in chunks and split into substrings using 
    // the configured delimiters
    template <typename DataFlow>
        requires std::derived_from<typename DataFlow::Type, std::istream>
    auto apply(DataFlow& flow) const {
        auto local_map = delim_map_;
        return flow.template derive<std::string, Container, Args...>(
            [local_map](auto& input, auto& output) {
                local_map->Cache();

                for (auto& stream : input) {
                    char buffer[DelimiterMap::kBufferSize];
                    std::string word;

                    while (stream) {
                        stream.read(buffer, DelimiterMap::kBufferSize);
                        for (int i = 0; i < stream.gcount(); ++i) {
                            if (local_map->map[static_cast<int>(buffer[i])]) {
                                output.push_back(word);
                                word.clear();
                            } else {
                                word += buffer[i];
                            }
                        }
                    }

                    output.push_back(word);
                }
            });
    }
};

// Splits an input stream into substrings by the list of delimiters
// passed as a constructor argument.
template <
    template <typename, typename...> typename Container = SameContainer,
    typename... Args>
inline auto Split(const std::string& delimiters) {
    return SplitAdapter<Container, Args...>(delimiters);
}

#endif  // _LABWORK8_LIB_SPLIT_HPP_

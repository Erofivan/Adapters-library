#ifndef _LABWORK8_LIB_SPLIT_EXPECTED_HPP
#define _LABWORK8_LIB_SPLIT_EXPECTED_HPP

#include "struct.hpp"

template <
    template <typename, typename...> typename Container,
    typename... Args>
class SplitExpectedAdapter {
 public:
    // Splits a data flow of std::expected<T, E> into two separate containers:
    // one container with all errors (E)
    // another container with all successful values (T)
    // This method processes the input data flow twice:
    // first extracting errors, then extracting values
    template <typename DataFlow>
    auto apply(DataFlow& data_flow) {
        // Extract all errors from the input stream
        auto error_stream = data_flow.template derive<
            typename DataFlow::Type::error_type,
            Container,
            Args...>(
            [](auto& input, auto& error_output) {
                for (auto& result : input) {
                    if (!result) {
                        error_output.push_back(result.error());
                    }
                }
            });

        // Extract all successful values from the input stream
        auto value_stream = data_flow.template derive<
            typename DataFlow::Type::value_type,
            Container,
            Args...>(
            [](auto& input, auto& value_output) {
                for (auto& result : input) {
                    if (result) {
                        value_output.push_back(result.value());
                    }
                }
            });

        return std::pair{error_stream, value_stream};
    }
};

// If the previous adapter returns std::expected, splits the processing pipeline
// into two branches: one for successful values and another for errors.
template <
    template <typename, typename...> typename Container = SameContainer,
    typename... Args>
inline auto SplitExpected() {
    return SplitExpectedAdapter<Container, Args...>();
}

#endif  // _LABWORK8_LIB_SPLIT_EXPECTED_HPP

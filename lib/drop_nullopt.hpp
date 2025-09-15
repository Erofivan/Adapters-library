#ifndef _LABWORK8_LIB_DROP_NULLOPT_
#define _LABWORK8_LIB_DROP_NULLOPT_

#include "struct.hpp"

template <
    template <typename, typename...> typename Container,
    typename... Args>
class DropNulloptAdapter {
public:
    virtual ~DropNulloptAdapter() = default;

    // This method iterates throw input container and checks whether current 
    // item has_value(). If so it will push back it to the output container.
    // Otherwise, item is skipped.
    template <typename DataFlow>
    auto apply(DataFlow& data_flow) const {
        return data_flow.template derive<
            typename DataFlow::Type::value_type,
            Container,
            Args...>(
            [](auto& input_container, auto& output_container) {
                for (const auto& maybe_value : input_container) {
                    if (maybe_value.has_value()) {
                        output_container.push_back(maybe_value.value());
                    }
                }
            });
    }
};

// Filters a stream of std::optional<T> by removing all std::nullopt values.
template <
    template <typename, typename...> typename Container = SameContainer,
    typename... Args>
inline auto DropNullopt() {
    return DropNulloptAdapter<Container, Args...>();
}

#endif  // _LABWORK8_LIB_DROP_NULLOPT_

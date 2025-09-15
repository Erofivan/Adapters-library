#ifndef _LABWORK8_LIB_FILTER_HPP_
#define _LABWORK8_LIB_FILTER_HPP_

#include <algorithm>
#include <iterator>

#include "struct.hpp"

template <
    typename Predicate,
    template <typename, typename...> typename Container,
    typename... Args>
class FilterAdapter {
public:
    virtual ~FilterAdapter() = default;

    explicit FilterAdapter(const Predicate& func)
        : predicate_func_(func) {}

    template <typename DataFlow>
    auto apply(DataFlow& data_flow) const {
        const Predicate& func = predicate_func_;

        return data_flow.template derive<
            typename DataFlow::Type,
            Container,
            Args...>(
            [func](auto& input_container, auto& output_container) {
                std::remove_copy_if(
                    input_container.begin(),
                    input_container.end(),
                    std::back_inserter(output_container),
                    [&func](auto& value) { return !func(value); });
            });
    }
private:
    Predicate predicate_func_;
};

// Filters elements from the input container using a provided predicate.
// Only elements for which the predicate returns true are passed through.
template <
    template <typename, typename...> typename Container = SameContainer,
    typename... Args,
    typename Predicate>
inline auto Filter(Predicate func) {
    return FilterAdapter<Predicate, Container, Args...>(func);
}

#endif  // _LABWORK8_LIB_FILTER_HPP_

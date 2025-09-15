#ifndef _LABWORK8_LIB_TRANSFORM_HPP_
#define _LABWORK8_LIB_TRANSFORM_HPP_

#include <algorithm>
#include <iterator>

#include "struct.hpp"

template <
    typename Function,
    template <typename, typename...> typename Container,
    typename... Args>
class TransformAdapter {
private:
    Function transform_func_; // User-provided function to apply to each element

public:
    virtual ~TransformAdapter() = default;

     // Construct the adapter by storing a copy of the transform function
    explicit TransformAdapter(const Function& func)
        : transform_func_(func) {}

    // Deduce the result type of applying func to an InputType
    // Ex.: if Function is int(double),
    // and InputType is double, then ResultType<double> is int
    template <typename InputType>
    using ResultType = decltype(
        transform_func_(std::declval<InputType&>()));

    // Applies the transformation to all elements in the input data flow. 
    template <typename DataFlow>
    auto apply(DataFlow& data_flow) const {
        const Function& func = transform_func_;

        return data_flow.template derive<
            ResultType<typename DataFlow::Type>,
            Container,
            Args...>(
            [func](auto& input_container, auto& output_container) {
                std::transform(input_container.begin(),
                               input_container.end(),
                               std::back_inserter(output_container),
                               func);
            });
    }
};

// Changes the values of elements, similar to how the std::transform
// does it by applying a given function to each element.
template <
    template <typename, typename...> typename Container = SameContainer,
    typename... Args,
    typename Function>
inline auto Transform(Function func) {
    return TransformAdapter<Function, Container, Args...>(func);
}

#endif  // _LABWORK8_LIB_TRANSFORM_HPP_

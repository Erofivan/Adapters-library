#ifndef _LABWORK8_LIB_AS_DATA_FLOW_HPP_
#define _LABWORK8_LIB_AS_DATA_FLOW_HPP_

#include <utility>

#include "pipeline.hpp"
#include "struct.hpp"

// Converts the container into a data stream for further processing
template <typename ValueType,
          template <typename, typename...> typename Container,
          typename... Args>
inline auto AsDataFlow(Container<ValueType, Args...>& container) {
    return Pipeline<ValueType, Container, Args...>(container);
}


// Converts the container into a data stream for further processing
template <typename ValueType,
          template <typename, typename...> typename Container,
          typename... Args>
inline auto AsDataFlow(
    const Container<ValueType, Args...>& container) {
    return Pipeline<const ValueType, Container, Args...>(container);
}


// Converts the container into a data stream for further processing
template <typename ValueType,
          template <typename, typename...> typename Container,
          typename... Args>
inline auto AsDataFlow(Container<ValueType, Args...>&& container) {
    return Pipeline<ValueType, Container, Args...>(std::move(container));
}

// creates a new container by copying from an existing source container,
// and wraps it into a data pipeline. Allows rebinding to a different container type.
template <template <typename, typename...> typename Container = SameContainer,
          typename... Args, 
		  typename ValueType,
          template <typename, typename...> typename SourceContainer,
          typename... SourceContainerArgs>
inline auto CopyAsDataFlow(
    const SourceContainer<ValueType, SourceContainerArgs...>& source_container) {
    using NewFlow = Pipeline<ValueType, SourceContainer, SourceContainerArgs...>::
        template Rebind<ValueType, Container, Args...>;

    NewFlow new_flow([&source_container](
                         typename NewFlow::Container& new_container) {
        new_container = typename NewFlow::Container(source_container.cbegin(),
                                                    source_container.cend());
    });

    new_flow.content();
    return new_flow;
}

#endif  // _LABWORK8_LIB_AS_DATA_FLOW_HPP_
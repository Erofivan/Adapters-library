#ifndef _LABWORK8_LIB_AS_VECTOR_HPP
#define _LABWORK8_LIB_AS_VECTOR_HPP

#include <vector>
/**
 * 
 * 
 */
template <
    template <typename, typename...> typename Container,
    typename... Args>
class AsContainer {
public:
    virtual ~AsContainer() = default;
    
    // Creates and returns a container with elements of type DataFlow::Type 
    // with all elements copied from data_flow
    template <typename DataFlow>
    Container<typename DataFlow::Type, Args...> apply(
        DataFlow& data_flow) const {
        return {data_flow.begin(), data_flow.end()};
    }
};
// Helper struct that creates std::vector adapters
struct AsVectorAdapter {
    // Creates a vector adapter without immediate conversion
    template <typename... Args>
    auto operator()() const {
        return AsContainer<std::vector, Args...>{};
    }
    // Immediately converts a data flow to std::vector
    // Returns a std::vector containing all elements from data_flow
    template <typename DataFlow>
    auto operator()(DataFlow& data_flow) const {
        return operator()<>().apply(data_flow);
    }
};

/**
 * Converts a container to a data stream for further processing.
 * 
*/
inline constexpr AsVectorAdapter AsVector; // Global adapter instance for convenient
                                           // vector conversion syntax:
                                           // auto vec = AsVector(data_flow);

#endif  // _LABWORK8_LIB_AS_VECTOR_HPP

#ifndef _LABWORK8_LIB_STRUCT_HPP_
#define _LABWORK8_LIB_STRUCT_HPP_

// Marker type used when you want the output container to be the same 
// as the input container.
template <typename TValue, typename ...Args>
struct SameContainer {};

// Concept that checks if a type Alloc behaves like a valid allocator for elements.
// It requires that:
// alloc.allocate(n) returns a pointer to value_type, and you can dereference it.
// alloc.deallocate(...) is callable with (pointer, size).
// Alloc is copy-constructible and equality-comparable.
template<class Alloc>
concept IsAllocator = requires(Alloc alloc, std::size_t n) {
    { *alloc.allocate(n) } -> std::same_as<typename Alloc::value_type&>;
    { alloc.deallocate(alloc.allocate(n), n) };  
}   && std::copy_constructible<Alloc>
    && std::equality_comparable<Alloc>;

// Primary template for mapping a container’s allocator 
// or other template parameter Alloc
// to a type suitable for rebind if Alloc is indeed an allocator. In the default case,
// we leave Alloc unchanged.
template <typename Alloc, template <typename, typename...> typename Container2, typename TValue2>
struct ConvertContainerParametrs {
	  using type = Alloc;
};

// Specialization: if Alloc satisfies IsAllocator, then rebind that allocator so that 
// it allocates elements of type TValue2 instead of whatever it originally did.
template <
    IsAllocator Alloc, 
    template <typename, typename...> typename Container2, 
    typename TValue2>
struct ConvertContainerParametrs<Alloc, Container2, TValue2> {
	  using type = std::allocator_traits<Alloc>::template rebind_alloc<TValue2>;
};

// Concept to test whether a given template parameter Alloc can be converted 
// into an allocator
// for Container2<TValue2> via ConvertContainerParametrs. If the nested ::type is valid,
// the concept is satisfied.
template <
    typename Alloc, 
    template <typename, typename...> typename Container2, 
    typename TValue2>
concept ContainerParamConvertible = requires {
	  typename ConvertContainerParametrs<Alloc, Container2, TValue2>::type;
};

// Forward declaration of Pipeline class.
// Pipeline<TValue, C, Args...> represents a data flow of elements of type TValue
// stored in a container C<TValue, Args...>. Its definition is elsewhere.
template <
    typename TValue, 
    template <typename, typename...> typename C,
    typename ...Args>
class Pipeline;

// ContainerTraits helps to take an existing sequence
// of template parameters (e.g., allocatorsor other container arguments)
// and transform them so that any allocator Alloc is rebound
// to allocate elements of type TValue2. 
// The result is a new Pipeline<TValue2, Container2, ...> type.
template <template<typename, typename...> typename Container2, typename TValue2>
struct ContainerTraits {
    // Helper to bundle Alloc parameter pack into a single type.
    template <typename ...Args_>
    struct ArgPack {};

    // Prepend a new type Alloc to an existing ArgPack<...> of types.
    template <typename Alloc, typename Pack>
    struct Prepend {};

    // Prepend a new type Alloc to an existing ArgPack<...> of types.
    template <typename Alloc, typename ...Args_>
    struct Prepend<Alloc, ArgPack<Args_...>> {
        using Type = ArgPack<Alloc, Args_...>;
    };

    // Default conversion: leave the pack as-is.
    template <typename Pack>
    struct Convert {
        using Type = Pack;
    };

    // If the first type in the pack (Args0) is not an allocator convertible to TValue2,
    // skip it and recurse on the rest of the pack.
    template <typename Args0, typename ... Args_>
    struct Convert<ArgPack<Args0, Args_...>> {
        using Type = Convert<ArgPack<Args_...>>::Type;
    };

    // If the first type in the pack (Args0) can
    // be converted to an allocator for TValue2,
    // then rebind that allocator and prepend it to
    // the conversion of the remaining pack.
    template <typename Args0, typename ... Args_>
      requires ContainerParamConvertible<Args0, Container2, TValue2>
    struct Convert<ArgPack<Args0, Args_...>> {
        using Type = Prepend<
            typename ConvertContainerParametrs<Args0, Container2, TValue2>::type,
            typename Convert<ArgPack<Args_...>>::Type
        >::Type;
    };

    // After converting all allocator parameters, wrap the resulting ArgPack<...> into
    // a Pipeline<TValue2, Container2, Args_...> type.
    template <typename Pack>
    struct ToFlow {};

    // After converting all allocator parameters, wrap the resulting ArgPack<...> into
    // a Pipeline<TValue2, Container2, Args_...> type.
    template <typename ...Args_>
    struct ToFlow<ArgPack<Args_...>>{
        using Type = Pipeline<TValue2, Container2, Args_...>;
    };

    // Public alias: given an initial list of container arguments Args2...,
    // Convert them to the correct allocator types (if any), then build the final
    // Pipeline<TValue2, Container2, ...> type.
    template <typename ...Args2>
    using Resolve = ToFlow<typename Convert<ArgPack<Args2...>>::Type>::Type;
};

#endif // _LABWORK8_LIB_STRUCT_HPP_
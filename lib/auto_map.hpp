#ifndef _LABWORK8_LIB_AUTO_MAP_HPP_
#define _LABWORK8_LIB_AUTO_MAP_HPP_

#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <stdexcept>

namespace auto_map {
    
// A minimal associative container that behaves like std::map or std::unordered_map,
// but is internally implemented using a std::vector. Optimized for small datasets.
template <class K, class V, class KeyEqual = std::equal_to<K>>
class VecMap final {
public:
    using value_type = std::pair<const K, V>;
    using data_type = std::vector<value_type>;
    using iterator = typename data_type::iterator;

    VecMap() = default;

    // Returns true if key exists in the container
    bool contains(const K& key) const {
        return where(key) != data_.end();
    }

    // Returns reference to value associated with key or throws if not found
    V& at(const K& key) {
        if (contains(key)) {
            return cached_iter_->second;
        }
        throw std::out_of_range("no element found");
    }

    // Inserts a key-value pair, returns iterator and insertion status
    std::pair<iterator, bool> insert(const value_type& pair) {
        if (contains(pair.first)) {
            return {cached_iter_, false};
        }

        data_.push_back(pair);
        cached_iter_ = std::prev(data_.end());
        return {cached_iter_, true};
    }

    iterator begin() { return data_.begin(); }
    iterator end() { return data_.end(); }

private:
    KeyEqual key_equal_; // Equality predicate to compare keys
    data_type data_; // Underlying storage
    mutable iterator cached_iter_ = data_.end(); // Cache last lookup

    // Locates an element with the given key. Uses cache for faster repeated lookup.
    iterator where(const K& key) const {
        if (cached_iter_ == data_.end() || !key_equal_(cached_iter_->first, key)) {
            cached_iter_ = std::find_if(
                const_cast<data_type&>(data_).begin(),
                const_cast<data_type&>(data_).end(),
                [&](const value_type& p) { return key_equal_(key, p.first); }
            );
        }
        return cached_iter_;
    }
};

// === Map specification detection (concepts, SFINAE, etc.) ===

// Empty tag struct used when no custom spec is provided
struct Empty {};

// Concept: checks whether a comparison function f(k1, k2) is valid
template <class F, class K>
concept IsCmp = requires(F f, K k1, K k2) {
    { f(k1, k2) } -> std::convertible_to<bool>;
};

// Primary selector template for choosing the underlying map type
template <class K, class V, class MapSpec = Empty>
struct AutoMapSelector {};

// Type resolution helper: default key equality is std::equal_to
template <class K, class MapSpec>
struct GetKeyEqual {
    using type = std::equal_to<K>;
};

// Specialization: use user-defined KeyEqual if it's a valid comparator
template <class K, class MapSpec>
    requires IsCmp<typename MapSpec::KeyEqual, K>
struct GetKeyEqual<K, MapSpec> {
    using type = typename MapSpec::KeyEqual;
};

// Concept: ensures a valid KeyEqual is available
template <class K, class MapSpec>
concept HasKeyEqual = requires(GetKeyEqual<K, MapSpec>::type f, K k1, K k2) {
    { f(k1, k2) } -> std::convertible_to<bool>;
};

// VecMap is selected if only KeyEqual is available and no other traits
template <class K, class V, class MapSpec>
    requires HasKeyEqual<K, MapSpec>
struct AutoMapSelector<K, V, MapSpec> {
    using type = VecMap<K, V, typename GetKeyEqual<K, MapSpec>::type>;
};

// Default comparator for ordered maps
template <class K, class MapSpec>
struct GetCompare {
    using type = std::less<K>;
};

// Use user-defined comparator if it's valid
template <class K, class MapSpec>
    requires IsCmp<typename MapSpec::Compare, K>
struct GetCompare<K, MapSpec> {
    using type = typename MapSpec::Compare;
};

// Concept: confirms whether operator< or Compare is available
template <class K, class MapSpec>
concept HasCompare =
    IsCmp<typename MapSpec::Compare, K> ||
    requires(K k1, K k2) {
        { k1 < k2 } -> std::convertible_to<bool>;
    };

// std::map is selected if a valid Compare is available
template <class K, class V, class MapSpec>
    requires HasCompare<K, MapSpec>
struct AutoMapSelector<K, V, MapSpec> {
    using type = std::map<K, V, typename GetCompare<K, MapSpec>::type>;
};

// If both Compare and KeyEqual are available, still prefer std::map
template <class K, class V, class MapSpec>
    requires HasKeyEqual<K, MapSpec> && HasCompare<K, MapSpec>
struct AutoMapSelector<K, V, MapSpec> {
    using type = std::map<K, V, typename GetCompare<K, MapSpec>::type>;
};

// Default hasher for unordered maps
template <class K, class MapSpec>
struct GetHash {
    using type = std::hash<K>;
};

// Specialization: uses user-defined hash function if it's convertible to std::function
template <class K, class MapSpec>
    requires std::convertible_to<typename MapSpec::Hash, std::function<size_t(K)>>
struct GetHash<K, MapSpec> {
    using type = typename MapSpec::Hash;
};

// Concept: checks if a callable returns size_t when passed a key
template <class K, class MapSpec>
concept HasHash = requires(GetHash<K, MapSpec>::type f, K k) {
    { f(k) } -> std::same_as<size_t>;
};

// unordered_map is selected if KeyEqual and Hash are both valid
template <class K, class V, class MapSpec>
    requires HasKeyEqual<K, MapSpec> && HasHash<K, MapSpec>
struct AutoMapSelector<K, V, MapSpec> {
    using type = std::unordered_map<
        K, 
        V, 
        typename GetHash<K, MapSpec>::type, 
        typename GetKeyEqual<K, MapSpec>::type>;
};

// unordered_map is still selected if Hash, Compare and KeyEqual all exist
template <class K, class V, class MapSpec>
    requires HasKeyEqual<K, MapSpec> && HasCompare<K, MapSpec> && HasHash<K, MapSpec>
struct AutoMapSelector<K, V, MapSpec> {
    using type = std::unordered_map<
        K, 
        V, 
        typename GetHash<K, MapSpec>::type, 
        typename GetKeyEqual<K, MapSpec>::type>;
};

// A smart wrapper that selects between VecMap, std::map, or std::unordered_map
//  depending on the availability of Compare, Hash, and KeyEqual 
//  in the given specification (MapSpec).
template <class K, class V, class MapSpec = Empty>
using AutoMap = typename AutoMapSelector<K, V, MapSpec>::type;

}  // namespace auto_map

#endif  // _LABWORK8_LIB_AUTO_MAP_HPP_

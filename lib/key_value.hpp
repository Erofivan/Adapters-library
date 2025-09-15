#ifndef _LABWORK8_LIB_KEY_VALUE_HPP_
#define _LABWORK8_LIB_KEY_VALUE_HPP_

#include <utility>
#include <functional>

template <typename KeyType, typename ValueType>
struct KV {
    KeyType key;
    ValueType value;

    const std::pair<KeyType, ValueType> operator()() {
        return {key, value};
    };

    operator std::pair<KeyType, ValueType>() {
        return {key, value};
    }

    operator const std::pair<KeyType, ValueType>() const {
        return {key, value};
    }

    bool operator==(const KV& other) const {
        return key == other.key && value == other.value;
    }
};

namespace std {
    template <typename KeyType, typename ValueType>
    struct hash<KV<KeyType, ValueType>> {
        size_t operator()(const KV<KeyType, ValueType>& kv) const {
            return hash<KeyType>()(kv.key) ^ hash<ValueType>()(kv.value);
        }
    };
}

#endif  // _LABWORK8_LIB_KEY_VALUE_HPP_
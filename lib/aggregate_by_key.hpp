#ifndef _LABWORK8_LIB_AGGREGATE_BY_KEY_HPP_
#define _LABWORK8_LIB_AGGREGATE_BY_KEY_HPP_

#include "auto_map.hpp"
#include "pipeline.hpp"
#include "struct.hpp"

template <
    typename MapSpec,
    typename SignType,
    typename Accumulator,
    typename KeyExtractor,
    template <typename, typename...> typename Container,
    typename... Args>
class AggregateByKeyAdapter {
private:
    SignType initial_value_;
    Accumulator accumulate_;
    KeyExtractor extract_key_;

    template <typename KeyType, typename ValueType>
    class KeyIndexMap {
    private:
        auto_map::AutoMap<KeyType, size_t, MapSpec> key_to_index_;
        SignType initial_value_;
        Accumulator accumulate_;
        KeyExtractor extract_key_;

    public:
        explicit KeyIndexMap(AggregateByKeyAdapter& parent)
            : initial_value_(parent.initial_value_),
              accumulate_(parent.accumulate_),
              extract_key_(parent.extract_key_) {}

        template <typename TargetContainer>
        void Aggregate(TargetContainer& container, ValueType& value) {
            KeyType key = extract_key_(value);
            size_t index;

            if (key_to_index_.contains(key)) {
                index = key_to_index_.at(key);
            } else {
                index = container.size();
                key_to_index_.insert({key, index});
                container.push_back(std::pair{key, initial_value_});
            }

            accumulate_(value, container[index].second);
        }
    };

public:
    AggregateByKeyAdapter(const SignType& initial_value,
                    const Accumulator& accumulate,
                    const KeyExtractor& extract_key)
        : initial_value_(initial_value),
          accumulate_(accumulate),
          extract_key_(extract_key) {}

    virtual ~AggregateByKeyAdapter() = default;

    template <typename DataFlow>
    using KeyType =
        decltype(std::declval<KeyExtractor>()(
            std::declval<typename DataFlow::Type>()));

    template <typename DataFlow>
    using ResultPair = std::pair<KeyType<DataFlow>, SignType>;

    template <typename DataFlow>
        requires std::convertible_to<Accumulator,
                     std::function<void(typename DataFlow::Type&,
                                        SignType&)>> &&
                 std::convertible_to<KeyExtractor,
                     std::function<KeyType<DataFlow>(
                         typename DataFlow::Type&)>>
    auto apply(DataFlow& dataflow) {
        auto key_index_map = std::make_shared<
            KeyIndexMap<KeyType<DataFlow>,
                        typename DataFlow::Type>>(*this);

        return dataflow.template derive<ResultPair<DataFlow>,
                                        Container,
                                        Args...>(
            [key_index_map](auto& input, auto& output) {
                for (auto& value : input) {
                    key_index_map->Aggregate(output, value);
                }
            });
    }
};

/**
 * Aggregates values relative to their corresponding keys.
 * The value associated with each key is updated using 
 * the provided functor — the aggregator.
 * 
 * The operation is eager, not lazy.
 */
template <
    typename MapSpec = auto_map::Empty,
    template <typename, typename...> typename Container = SameContainer,
    typename... Args,
    typename SignType,
    typename Accumulator,
    typename KeyExtractor>
inline auto AggregateByKey(SignType initial_value,
                           Accumulator accumulate,
                           KeyExtractor extract_key) {
    return AggregateByKeyAdapter<MapSpec,
                           SignType,
                           Accumulator,
                           KeyExtractor,
                           Container,
                           Args...>(
        initial_value, accumulate, extract_key);
}

#endif  // _LABWORK8_LIB_AGGREGATE_BY_KEY_HPP_

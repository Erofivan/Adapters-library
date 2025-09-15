#ifndef _LABWORK8_LIB_JOIN_HPP_
#define _LABWORK8_LIB_JOIN_HPP_

#include <memory>
#include <vector>

#include "auto_map.hpp"
#include "join_result.hpp"
#include "key_value.hpp"
#include "struct.hpp"

// Template parameters holder for JoinAdapter
template <typename MapSpec,
          template <typename, typename...> typename Container,
          typename... Args>
struct JoinTemplateHolder {};

// Main join adapter class implementing hash join logic
template <typename MapSpec,
          typename RightFlow,
          typename GetRightKey,
          typename GetLeftKey,
          typename GetRightValue,
          typename GetLeftValue,
          template <typename, typename...> typename Container,
          typename... Args>
class JoinAdapter {
public:
 	// Deducing key and value types from right flow extractors
	using RightKey = decltype(
    	std::declval<GetRightKey>()(std::declval<typename RightFlow::Type>()));
  	using RightValue = decltype(
      	std::declval<GetRightValue>()(std::declval<typename RightFlow::Type>()));

private:
	// Class that manages right-side data caching and lookup
  	class RightMap {
   	public:
		RightFlow right_flow;
		GetRightKey get_right_key;
		GetRightValue get_right_value;

		 // Initializis with data source and extraction functions
		explicit RightMap(const RightFlow& flow,
						const GetRightKey& get_key,
						const GetRightValue& get_value)
			: right_flow(flow),
			get_right_key(get_key),
			get_right_value(get_value) {}

		// Prepares data access before join operation
		void Initialize() { 
			getter_ = right_flow.CurrentStateGetter(); 
		}

		// Builds lookup map lazily on first access
		void EnsureCache() {
			if (cached_) {
				return;
			}
			cached_ = true;
			// Process all right items into multimap
			for (auto& item : getter_()) {
				const auto& key = get_right_key(item);
				const auto& value = get_right_value(item);
				map_.insert({key, std::vector<RightValue>{}});
				map_.at(key).push_back(value);
			}
		}

		// Returns all values matching key (empty vector if none)
		std::vector<RightValue>& Values(const RightKey& key) {
			EnsureCache();
			if (map_.contains(key)) return map_.at(key);
			return dummy_;
		}

	private:
		bool cached_ = false;
		std::vector<RightValue> dummy_; // Empty result placeholder
		auto_map::AutoMap<RightKey, std::vector<RightValue>> map_; // Multimap storage
		typename RightFlow::Getter getter_; // Data accessor
  	};

	// Bundles left-side extraction functions
  	struct LeftFuncs {
		GetLeftKey get_left_key;
		GetLeftValue get_left_value;

		LeftFuncs(const GetLeftKey& get_key, const GetLeftValue& get_value)
			: get_left_key(get_key), get_left_value(get_value) {}
 	};

	std::shared_ptr<RightMap> right_map_ptr_;  // Shared right data states
	std::shared_ptr<LeftFuncs> left_funcs_ptr_;// Shared left operations

public:
 	// Deduce left key type for given flow
	template <typename LeftFlow>
	using LeftKey = decltype(left_funcs_ptr_->get_left_key(
		std::declval<typename LeftFlow::Type>()));
	
	// Deduce left value type for given flow
	template <typename LeftFlow>
	using LeftValue = decltype(left_funcs_ptr_->get_left_value(
		std::declval<typename LeftFlow::Type>()));

	// Result type for joined elements
	template <typename LeftFlow>
	using Result = JoinResult<LeftValue<LeftFlow>, RightValue>;

  	virtual ~JoinAdapter() = default;

	// Constructs adapter with right flow and extraction functions
	JoinAdapter(JoinTemplateHolder<MapSpec, Container, Args...>,
			const RightFlow& right_flow,
			GetRightKey get_right_key,
			GetLeftKey get_left_key,
			GetRightValue get_right_value,
			GetLeftValue get_left_value)
		: right_map_ptr_(std::make_shared<RightMap>(
				right_flow, get_right_key, get_right_value)),
			left_funcs_ptr_(
				std::make_shared<LeftFuncs>(get_left_key, get_left_value)) {}


	// Applies join operation to left flow
	template <typename LeftFlow>
	auto apply(LeftFlow& left_flow) {
		using ThisLeftFlow = LeftFlow;
		using ThisResult = Result<ThisLeftFlow>;
		
		auto& right_map_ptr = right_map_ptr_;
		auto& left_funcs_ptr = left_funcs_ptr_;

		right_map_ptr->Initialize(); // Prepare right data

		// Create derived flow for join results
		auto next_flow = left_flow.template derive<ThisResult, std::vector>(
            [right_map_ptr, left_funcs_ptr](auto& left_container, auto& new_container) {
                for (auto& left_item : left_container) {
                    const auto& left_key =
                        left_funcs_ptr->get_left_key(left_item);
                    const auto& left_value =
                        left_funcs_ptr->get_left_value(left_item);

                    auto& right_values = right_map_ptr->Values(left_key);
                    if (right_values.empty()) {
                        new_container.push_back(ThisResult{left_value, std::nullopt});
                    } else {
                        for (auto& right_value : right_values) {
                            new_container.push_back(ThisResult{left_value, right_value});
                        }
                    }
                }
            });
		
		// Link right flow to derived flow
		right_map_ptr->right_flow.SetLastDerived(next_flow);
		return next_flow;
	}
	
};

// Performs a join operation between two data flows using specified key and
// value extractors. Matches left and right values by keys.
template <typename MapSpec = auto_map::Empty,
          template <typename, typename...> typename Container = SameContainer,
          typename... Args,
          typename RightFlow>
inline auto Join(RightFlow& right_flow) {
	return JoinAdapter(JoinTemplateHolder<MapSpec, Container, Args...>(), right_flow,
				[](const auto& kv) { return kv.key; },
				[](const auto& kv) { return kv.key; },
				[](const auto& kv) { return kv.value; },
				[](const auto& kv) { return kv.value; });
}

// Performs a join operation between two data flows using specified key extractors
template <typename MapSpec = auto_map::Empty,
          template <typename, typename...> typename Container = SameContainer,
          typename... Args,
          typename RightFlow,
          typename GetLeftKey,
          typename GetRightKey>
inline auto Join(RightFlow& right_flow,
                 const GetLeftKey& get_left_key,
                 const GetRightKey& get_right_key) {
	return JoinAdapter(JoinTemplateHolder<MapSpec, Container, Args...>(), right_flow,
				get_right_key,
				get_left_key,
				[](const auto& v) { return v; },
				[](const auto& v) { return v; });
}

#endif  // _LABWORK8_LIB_JOIN_HPP_
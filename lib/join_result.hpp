#ifndef _LABWORK8_LIB_JOIN_RESULT_HPP_
#define _LABWORK8_LIB_JOIN_RESULT_HPP_

#include <optional>

// JoinResult represents the result of a join operation, holding data from
// both streams.
template <typename Base, typename Joined>
struct JoinResult {
	Base base;
	std::optional<Joined> joined;
 
	bool operator==(const JoinResult&) const = default;
};

#endif
#ifndef _LABWORK8_LIB_OUT_HPP_
#define _LABWORK8_LIB_OUT_HPP_

#include <ostream>

#include "write.hpp"

class OutAdapter : public WriteAdapter {
public:
    explicit OutAdapter(std::ostream& output)
    : 
		WriteAdapter(output, '\n')
	{}
};

// Outputs data to a given output stream, appending newline characters.
inline auto Out(std::ostream& output) {
    return OutAdapter(output);
}

#endif  // _LABWORK8_LIB_OUT_HPP_

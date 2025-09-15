#ifndef _LABWORK8_LIB_WRITE_HPP_
#define _LABWORK8_LIB_WRITE_HPP_

#include <ostream>
#include <string>

class WriteAdapter {
public:
    virtual ~WriteAdapter() = default;

    explicit WriteAdapter(std::ostream& output_stream, std::string delimiter)
        : output_stream_(output_stream),
          delimiter_(std::move(delimiter)) 
        {}

    WriteAdapter(std::ostream& output_stream, char delimiter_char)
        : WriteAdapter(output_stream, std::string(1, delimiter_char)) {}

    // Outputs all value content from flow into output stream via given delimeter 
    template <typename DataFlow>
    DataFlow& apply(DataFlow& data_flow) {
        for (const auto& value : data_flow.content()) {
            output_stream_ << value << delimiter_;
        }
        return data_flow;
    }

private:
    std::ostream& output_stream_;
    std::string delimiter_;
};

// Writes all elements from the input container to a stream,
// separating them with a given delimiter.
template <typename Separator>
inline auto Write(std::ostream& output_stream, Separator delimiter) {
    return WriteAdapter(output_stream, delimiter);
}

#endif  // _LABWORK8_LIB_WRITE_HPP_

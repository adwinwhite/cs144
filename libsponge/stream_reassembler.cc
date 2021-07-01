#include "stream_reassembler.hh"
#include <tuple>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : intervals_(vector<tuple<uint64_t, uint64_t>>(1, make_tuple(0, 0))), stream_(ByteStream(capacity)), capacity_(capacity), eof_(false), final_index(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    //First check whether there is overlapping
    for (auto &t : intervals_) {
        if (index > get<0>(t) && index < get<1>(t)) {
            return;
        }
    }

    //Since there is no overlapping, we can try to write data to buffer.
    //Find out where should we write.
    if (index == get<1>(intervals_[0])) {
        size_t written_len = stream_.write(data);
        get<1>(intervals_[0]) = get<1>(intervals_[0]) + written_len;
    } else {
        size_t written_len = stream_.write(data, index - get<1>(intervals_[0]));
        intervals_.push_back(std::make_tuple(index, index + written_len));
    }
    if (eof) {
        eof_ = true;
        final_index = index + data.size();
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t count = 0;
    for (auto it = intervals_.cbegin() + 1; it != intervals_.cend(); ++it) {
        count += get<1>(*it) - get<0>(*it);
    }
    return count;
}

bool StreamReassembler::empty() const {
    return unassembled_bytes() == 0;
}

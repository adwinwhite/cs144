#include "stream_reassembler.hh"
#include <algorithm>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`



using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : intervals_(vector<uint64_t>(2, 0)), stream_(ByteStream(capacity)), eof_(false), final_index(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {

    //First check whether there is overlapping
    //It seems I got it wrong. overlapping is allowed? yes.

    //Write to get actuall length
    //Find out where to write according to index
    size_t new_size = index - intervals_[0];
    //If there is something to write
    if (new_size < stream_.total_capacity() && index + data.size() > intervals_[1]) {
        size_t actual_len;
        if (new_size > stream_.total_capacity() - stream_.remaining_capacity()) {
            //Do not count bytes written here. Not assembled, just stored.
            stream_.get_buffer().resize(new_size, false);
            actual_len = stream_.write(data, false);
        } else {
            actual_len = stream_.write(data.substr(intervals_[1] - index));
        }
        //Recompute intervals
        //Filter out interval contained by data
        intervals_.erase(remove_if(begin(intervals_), end(intervals_), [index, actual_len](uint64_t x){ return x >= index && x <= index + actual_len; }), end(intervals_));
        intervals_.push_back(index);
        intervals_.push_back(index + actual_len);
        sort(intervals_.begin(), intervals_.end());
        size_t left_ind_in_intervals = find(intervals_.begin(), intervals_.end(), index) - intervals_.begin();
        if (left_ind_in_intervals % 2 != 0) {
            intervals_.erase(intervals_.begin() + left_ind_in_intervals - 1);
        } else {
            size_t right_ind_in_intervals = find(intervals_.begin(), intervals_.end(), index + actual_len) - intervals_.begin();
            if (intervals_.size() % 2 != 0) {
                intervals_.erase(intervals_.begin() + right_ind_in_intervals + 1);
            }
        }
    }


    //To be honest, I have no idea how eof works here,
    //Should I end input once receiving an eof? Or when obtaining a complete stream?
    //If there is no byte written, should the eof count?
    if (eof) {
        eof_ = true;
        final_index = index + data.size();
        stream_.end_input();
    }

}

size_t StreamReassembler::unassembled_bytes() const {
    size_t count = 0;
    for (auto it = intervals_.cbegin() + 2; it != intervals_.cend(); it += 2) {
        count += (*(it+1)) - (*it);
    }
    return count;
}

bool StreamReassembler::empty() const {
    return unassembled_bytes() == 0;
}

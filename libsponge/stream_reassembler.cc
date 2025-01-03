#include "stream_reassembler.hh"
#include <algorithm>
#include <iostream>



// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`



using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : intervals_(vector<uint64_t>(1, 0)), stream_(ByteStream{capacity}), end_index_(0), eof_received_(false) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    //What if the first string does not start at 0?
    //When does it become at eof?
    //When reading if all bytes are read and eof has been received.

    //read and push both change intervals_
    //and we have no way of controlling reading
    //keep them in sync here
    //intervals_[0] == bytes_read
    //intervals_[1] == bytes_written
    // intervals_[0] = stream_.bytes_read();



    //OUT: new_intervals, new_buffer, new_num_bytes_written

    //No need to do anything if data is already written
    if (index + data.size() < stream_.bytes_written()) {
        return;
    }

    //Check whether eof is received
    //IN: eof, data, index
    //OUT: eof_received_, end_index_
    if ((!eof_received_) && eof) {
        eof_received_ = true;
        end_index_ = index + data.size();
    }

    //Truncate exceeded head and tail data
    //OUT: actual_index, actual_data
    size_t actual_index = index;
    string actual_data = data;
    if (index < stream_.bytes_written()) {
        actual_index = stream_.bytes_written();
        actual_data = data.substr(stream_.bytes_written() - index);
    }
    if (actual_index + actual_data.size() > stream_.bytes_read() + stream_.total_capacity()) {
        actual_data = actual_data.substr(0, stream_.bytes_read() + stream_.total_capacity() - actual_index);
    }


    //Find intervals contained by data
    //IN: index, actual_len, intervals_
    //OUT: indices of nearest boundaries inclusively
    size_t ind_left = intervals_.size();
    size_t ind_right = 0;
    for (size_t i = 0; i < intervals_.size(); ++i) {
        if (intervals_[i] >= actual_index) {
            ind_left = i;
            break;
        }
    }
    for (size_t i = intervals_.size(); i != 0; --i) {
        if (intervals_[i - 1] <= actual_index + actual_data.size()) {
            ind_right = i - 1;
            break;
        }
    }

    //Check whether tail is extended
    //IN: index, actual_data, contained_intervals, stream
    //OUT: new_actual_data
    if (intervals_.size() > 0) {
        if (ind_right % 2 == 1) {
            actual_data += stream_.get_buffer().peek(intervals_[ind_right + 1] - (actual_index + actual_data.size()), actual_index + actual_data.size() - stream_.bytes_read());
        }
    }

    //Write actual_data to buffer
    //IN: actual_data, buffer, num_bytes_written
    //OUT: new_buffer, num_bytes_written
    if (actual_index == stream_.bytes_written()) {
        stream_.write(actual_data);
    } else {
        stream_.get_buffer().resize(actual_index - stream_.bytes_read(), false);
        stream_.write(actual_data, false);
        stream_.get_buffer().resize(stream_.bytes_written() - stream_.bytes_read(), false);
    }

    //Recompute intervals
    //IN: intervals_, nearest boundaries
    //OUT: new_intervals
    if (ind_right % 2 == 0) {
        intervals_.insert(intervals_.begin() + ind_right + 1, actual_index + actual_data.size());
    }
    intervals_.erase(intervals_.begin() + ind_left, intervals_.begin() + ind_right + 1);
    if (ind_left % 2 == 1) {
        intervals_.insert(intervals_.begin() + ind_left, actual_index);
    }

    // intervals_.erase(remove_if(begin(intervals_), end(intervals_), [actual_index, actual_len](uint64_t x){ return x >= actual_index && x <= actual_index + actual_len; }), end(intervals_));

    //To be honest, I have no idea how eof works here,
    //Should I end input once receiving an eof? Or when obtaining a complete stream?
    //If there is no byte written, should the eof count?
    if (eof_received_ && stream_.bytes_written() == end_index_) {
        //End input only when all data are written and eof is true
        stream_.end_input();
    }

}

size_t StreamReassembler::unassembled_bytes() const {
    size_t count = 0;
    for (auto it = intervals_.cbegin() + 1; it != intervals_.cend(); it += 2) {
        count += (*(it+1)) - (*it);
    }
    return count;
}

bool StreamReassembler::empty() const {
    return unassembled_bytes() == 0;
}

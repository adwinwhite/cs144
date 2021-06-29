#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

// ByteStream::ByteStream(const size_t capacity) {
    // unread_start = 0;
    // unread_end = 0;
    // buffer = string(capacity, ' ');
    // num_bytes_written = 0;
    // num_bytes_read = 0;
    // _ended = false;
    // _error = false;
    // _full = false;
// }
size_t ByteStream::write(const string &data) {
    size_t len = _write(data);
    num_bytes_written += len;
    if (buffer_size() != total_capacity()) {
        used_buffer_size += len;
        if (buffer_size() > total_capacity()) {
            used_buffer_size = total_capacity();
        }
    }
    return len;
}

size_t ByteStream::_write(const string &data) {
    if (error()) {
        return 0;
    }
    if (input_ended()) {
        set_error();
        return 0;
    }
    if (_full) {
        return 0;
    }
    if (unread_end >= unread_start) {
        if (data.size() < buffer.size() - unread_end) {
            buffer.replace(unread_end, data.size(), data);
            unread_end += data.size();
            return data.size();
        } else {
            size_t first_written_size = buffer.size() - unread_end;
            // buffer.replace(unread_end, first_written_size, data.substr(0, first_written_size));
            buffer.replace(unread_end, first_written_size, data);
            if (data.size() - first_written_size < unread_start) {
                // buffer.replace(0, data.size() - first_written_size, data.substr(first_written_size, data.size() - first_written_size));
                buffer.replace(0, data.size() - first_written_size, data);
                unread_end = data.size() - first_written_size;
                return data.size();
            } else {
                // buffer.replace(0, unread_start, data.substr(first_written_size, first_written_size + unread_start));
                buffer.replace(0, unread_start, data);
                unread_end = unread_start;
                _full = true;
                return first_written_size + unread_start;
            }
        }
    } else {
        size_t vacant_length = unread_start - unread_end;
        if (data.size() < vacant_length) {
            buffer.replace(unread_end, data.size(), data);
            unread_end += data.size();
            return data.size();
        } else {
            buffer.replace(unread_end, vacant_length, data);
            unread_end = unread_start;
            _full = true;
            return vacant_length;
        }
    }
}


size_t ByteStream::remaining_capacity() const {
    if (this->is_full()) {
        return 0;
    }
    if (unread_end >= unread_start) {
        return buffer.size() - (unread_end - unread_start);
    } else {
        return unread_start - unread_end;
    }
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    // if (len == 0) {
        // return string("");
    // }
    size_t actual_len = len > buffer.size() - this->remaining_capacity() ? buffer.size() - this->remaining_capacity() : len;
    if (unread_end >= unread_start) {
        return buffer.substr(unread_start, actual_len);
    } else{
        size_t tail_size = buffer.size() - unread_start;
        if (actual_len > tail_size) {
            return buffer.substr(unread_start, tail_size) + buffer.substr(0, actual_len - tail_size);
        } else {
            return buffer.substr(unread_start, actual_len);
        }
    }
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    _full = len > 0 ? false : _full;
    if (unread_end >= unread_start) {
        if (len >= unread_end - unread_start) {
            unread_start = unread_end;
        } else {
            unread_start += len;
        }
    } else {
        size_t tail_size = buffer.size() - unread_start;
        if (len >= tail_size) {
            if (len - tail_size >= unread_end) {
                unread_start = unread_end;
            } else {
                unread_start = len - tail_size;
            }
        } else {
            unread_start += len;
        }
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    std::string read_string = _read(len);
    num_bytes_read += read_string.size();
    return read_string;
}

std::string ByteStream::_read(const size_t len) {
    _full = len > 0 ? false : _full;
    std::string read_string;
    if (unread_end >= unread_start) {
        if (len >= unread_end - unread_start) {
            read_string = buffer.substr(unread_start, unread_end - unread_start);
            unread_start = unread_end;
        } else {
            read_string = buffer.substr(unread_start, len);
            unread_start += len;
        }
    } else {
        size_t tail_size = buffer.size() - unread_start;
        if (len >= tail_size) {
            if (len - tail_size >= unread_end) {
                read_string = buffer.substr(unread_start, tail_size) + buffer.substr(0, unread_end);
                unread_start = unread_end;
            } else {
                read_string = buffer.substr(unread_start, tail_size) + buffer.substr(0, len - tail_size);
                unread_start = len - tail_size;
            }
        } else {
            read_string = buffer.substr(unread_start, len);
            unread_start += len;
        }
    }
    return read_string;
}


bool ByteStream::input_ended() const { return _ended; }

size_t ByteStream::buffer_size() const { return used_buffer_size; }

bool ByteStream::buffer_empty() const { return unread_start == unread_end && !this->is_full(); }

bool ByteStream::eof() const { return _ended && unread_end == unread_start; }

size_t ByteStream::bytes_written() const { return num_bytes_written; }

size_t ByteStream::bytes_read() const { return num_bytes_read; }

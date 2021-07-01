#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`


using namespace std;

size_t ByteStream::write(const string &data, const size_t offset) {
    if (input_ended() || error()) {
        return 0;
    }
    size_t len = buf_.write(data, offset);
    num_bytes_written += len;
    return len;
}



size_t ByteStream::remaining_capacity() const {
    return buf_.capacity() - buf_.used_size();
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    return buf_.peek(len);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t popped_len = buf_.pop(len);
    num_bytes_read += popped_len;
}


//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    std::string read_string = buf_.read(len);
    num_bytes_read += read_string.size();
    return read_string;
}

void ByteStream::end_input() { ended_ = true; }

bool ByteStream::input_ended() const { return ended_; }

size_t ByteStream::buffer_size() const { return buf_.used_size(); }

bool ByteStream::buffer_empty() const { return buf_.empty(); }

bool ByteStream::eof() const { return ended_ && buf_.empty(); }

bool ByteStream::error() const { return error_; }

size_t ByteStream::bytes_written() const { return num_bytes_written; }

size_t ByteStream::bytes_read() const { return num_bytes_read; }

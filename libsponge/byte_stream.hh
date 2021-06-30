#ifndef SPONGE_LIBSPONGE_BYTE_STREAM_HH
#define SPONGE_LIBSPONGE_BYTE_STREAM_HH

#include <string>

//! \brief An in-order byte stream.

//! Bytes are written on the "input" side and read from the "output"
//! side.  The byte stream is finite: the writer can end the input,
//! and then no more bytes can be written.
class ByteStream {
  private:
    size_t tot_capacity;
    size_t used_buffer_size;
    size_t unread_start;
    size_t unread_end;
    std::string buffer;
    size_t num_bytes_read;
    size_t num_bytes_written;
    bool _ended{};
    bool _full{};

    // Your code here -- add private members as necessary.

    // Hint: This doesn't need to be a sophisticated data structure at
    // all, but if any of your tests are taking longer than a second,
    // that's a sign that you probably want to keep exploring
    // different approaches.

    bool _error{};  //!< Flag indicating that the stream suffered an error.

    size_t _write(const std::string &data);
    std::string _read(const size_t len);
    size_t _pop(const size_t len);

  public:
    //! Construct a stream with room for `capacity` bytes.
    ByteStream(const size_t capacity) : tot_capacity(capacity), used_buffer_size(0), unread_start(0), unread_end(0), buffer(std::string(capacity, ' ')), num_bytes_read(0), num_bytes_written(0), _ended(false), _full(false), _error(false) {};

    //! \name "Input" interface for the writer
    //!@{

    //! Write a string of bytes into the stream. Write as many
    //! as will fit, and return how many were written.
    //! \returns the number of bytes accepted into the stream
    size_t write(const std::string &data);

    //! \returns the number of additional bytes that the stream has space for
    size_t remaining_capacity() const;

    size_t total_capacity() const { return tot_capacity; }

    //! Signal that the byte stream has reached its ending
    void end_input() { _ended = true; }

    //! Indicate that the stream suffered an error.
    void set_error() { _error = true; }

    bool is_full() const;
    //!@}

    //! \name "Output" interface for the reader
    //!@{

    //! Peek at next "len" bytes of the stream
    //! \returns a string
    std::string peek_output(const size_t len) const;

    //! Remove bytes from the buffer
    void pop_output(const size_t len);

    //! Read (i.e., copy and then pop) the next "len" bytes of the stream
    //! \returns a string
    std::string read(const size_t len);

    //! \returns `true` if the stream input has ended
    bool input_ended() const;

    //! \returns `true` if the stream has suffered an error
    bool error() const { return _error; }

    //! \returns the maximum amount that can currently be read from the stream
    size_t buffer_size() const;

    //! \returns `true` if the buffer is empty
    bool buffer_empty() const;

    //! \returns `true` if the output has reached the ending
    bool eof() const;
    //!@}

    //! \name General accounting
    //!@{

    //! Total number of bytes written
    size_t bytes_written() const;

    //! Total number of bytes popped
    size_t bytes_read() const;
    //!@}
};

#endif  // SPONGE_LIBSPONGE_BYTE_STREAM_HH

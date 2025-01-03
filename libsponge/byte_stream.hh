#ifndef SPONGE_LIBSPONGE_BYTE_STREAM_HH
#define SPONGE_LIBSPONGE_BYTE_STREAM_HH


#include "circular_buffer.hh"
//! \brief An in-order byte stream.

//! Bytes are written on the "input" side and read from the "output"
//! side.  The byte stream is finite: the writer can end the input,
//! and then no more bytes can be written.
class ByteStream {
  private:
    CircularBuffer buf_;
    size_t num_bytes_read;
    size_t num_bytes_written;
    bool ended_{};

    bool error_{};  //!< Flag indicating that the stream suffered an error.
    // bool eof_{};


  public:
    //! Construct a stream with room for `capacity` bytes.
    ByteStream(const size_t capacity) : buf_(capacity), num_bytes_read(0), num_bytes_written(0), ended_(false), error_(false) {};

    CircularBuffer& get_buffer();

    //! \name "Input" interface for the writer
    //!@{

    //! Write a string of bytes into the stream. Write as many
    //! as will fit, and return how many were written.
    //! \returns the number of bytes accepted into the stream
    size_t write(const std::string &data, const bool count = true);

    //! \returns the number of additional bytes that the stream has space for
    size_t remaining_capacity() const;

    //! Signal that the byte stream has reached its ending
    void end_input();

    //! Indicate that the stream suffered an error.
    void set_error();


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
    bool error() const;

    //! \returns the maximum amount that can currently be read from the stream
    size_t buffer_size() const;

    //! \returns `true` if the buffer is empty
    bool buffer_empty() const;

    //! \returns `true` if the output has reached the ending
    //



    bool eof() const;
    //!@}

    //! \name General accounting
    //!@{

    //! Total number of bytes written
    size_t bytes_written() const;

    void _offset_bytes_written(const int64_t num);

    //! Total number of bytes popped
    size_t bytes_read() const;
    //!@}
    //
    size_t total_capacity() const;
};

#endif  // SPONGE_LIBSPONGE_BYTE_STREAM_HH

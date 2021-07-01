#ifndef SPONGE_LIBSPONGE_CIRCULAR_buf__HH
#define SPONGE_LIBSPONGE_CIRCULAR_buf__HH

#include <string>

class CircularBuffer {
  private:
    size_t max_size_;
    size_t tail_;
    size_t head_;
    std::string buf_;
    bool full_{};


  public:
    CircularBuffer(const size_t size) : max_size_(size), tail_(0), head_(0), buf_(std::string(size, ' ')),  full_(false) {};

    size_t write(const std::string &data, const size_t offset = 0);

    std::string read(const size_t len);

    std::string peek(const size_t len) const;

    size_t pop(const size_t len);

    size_t capacity() const;


    bool full() const;

    size_t used_size() const;

    bool empty() const;
};

#endif

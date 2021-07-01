#include "circular_buffer.hh"


using namespace std;


// void CircularBuffer::set_head(const size_t head) {
    // head_ = head;
// }

// void CircularBuffer::set_tail(const size_t tail) {
    // tail_ = tail;
// }

void CircularBuffer::resize(const size_t new_size, const bool become_full) {
    head_ = (tail_ + new_size) % max_size_;
    full_ = become_full;
}



size_t CircularBuffer::write(const string &data) {
    if (full()) {
        return 0;
    }

    size_t written_len;
    if (head_ >= tail_) {
        if (data.size() < buf_.size() - head_) {
            buf_.replace(head_, data.size(), data);
            head_ += data.size();
            written_len = data.size();
        } else {
            size_t first_written_size = buf_.size() - head_;
            buf_.replace(head_, first_written_size, data.substr(0, first_written_size));
            if (data.size() - first_written_size < tail_) {
                buf_.replace(0, data.size() - first_written_size, data.substr(first_written_size, data.size() - first_written_size));
                head_ = data.size() - first_written_size;
                written_len = data.size();
            } else {
                buf_.replace(0, tail_, data.substr(first_written_size, tail_));
                head_ = tail_;
                full_ = true;
                written_len = first_written_size + tail_;
            }
        }
    } else {
        size_t vacant_length = tail_ - head_;
        if (data.size() < vacant_length) {
            buf_.replace(head_, data.size(), data);
            head_ += data.size();
            written_len = data.size();
        } else {
            buf_.replace(head_, vacant_length, data.substr(0, vacant_length));
            head_ = tail_;
            full_ = true;
            written_len = vacant_length;
        }
    }
    return written_len;
}
//offset is relative to tail.
// size_t CircularBuffer::write(const string &data, const int64_t offset) {
    // if (full()) {
        // return 0;
    // }

    // //Use fake_head to avoid changing real head
    // size_t fake_head = (head_ + offset) % max_size_;
    // size_t written_len;

    // if (fake_head >= tail_) {
        // if (data.size() < buf_.size() - fake_head) {
            // buf_.replace(fake_head, data.size(), data);
            // fake_head += data.size();
            // written_len = data.size();
        // } else {
            // size_t first_written_size = buf_.size() - fake_head;
            // buf_.replace(fake_head, first_written_size, data.substr(0, first_written_size));
            // if (data.size() - first_written_size < tail_) {
                // buf_.replace(0, data.size() - first_written_size, data.substr(first_written_size, data.size() - first_written_size));
                // fake_head = data.size() - first_written_size;
                // written_len = data.size();
            // } else {
                // buf_.replace(0, tail_, data.substr(first_written_size, tail_));
                // fake_head = tail_;
                // full_ = true;
                // written_len = first_written_size + tail_;
            // }
        // }
    // } else {
        // size_t vacant_length = tail_ - fake_head;
        // if (data.size() < vacant_length) {
            // buf_.replace(fake_head, data.size(), data);
            // fake_head += data.size();
            // written_len = data.size();
        // } else {
            // buf_.replace(fake_head, vacant_length, data.substr(0, vacant_length));
            // fake_head = tail_;
            // full_ = true;
            // written_len = vacant_length;
        // }
    // }
    // // Do not change head_ if there is offset.
    // head_ = offset == 0 ? fake_head : head_;
    // return written_len;
// }





string CircularBuffer::peek(const size_t len) const {
    // When there is nothing to pop, just return 0.
    if (empty() || len == 0) {
        return "";
    }

    std::string read_string;
    if (head_ > tail_) {
        if (len > head_ - tail_) {
            read_string = buf_.substr(tail_, head_ - tail_);
        } else {
            read_string = buf_.substr(tail_, len);
        }
    } else {
        size_t tail_size = buf_.size() - tail_;
        if (len >= tail_size) {
            if (len - tail_size >= head_) {
                read_string = buf_.substr(tail_, tail_size) + buf_.substr(0, head_);
            } else {
                read_string = buf_.substr(tail_, tail_size) + buf_.substr(0, len - tail_size);
            }
        } else {
            read_string = buf_.substr(tail_, len);
        }
    }
    return read_string;
}


size_t CircularBuffer::pop(const size_t len) {
    // When there is nothing to pop, just return 0.
    if (empty() || len == 0) {
        return 0;
    }

    // So now the buf_ is not empty. Popping will make it not full.
    full_ = false;

    size_t actual_len;
    if (head_ > tail_) {
        if (len > head_ - tail_) {
            actual_len = head_ - tail_;
            tail_ = head_;
        } else {
            actual_len = len;
            tail_ += len;
        }
    } else {
        size_t tail_size = buf_.size() - tail_;
        if (len >= tail_size) {
            if (len - tail_size >= head_) {
                actual_len = tail_size + head_;
                tail_ = head_;
            } else {
                actual_len = len;
                tail_ = len - tail_size;
            }
        } else {
            actual_len = len;
            tail_ += len;
        }
    }

    return actual_len;
}


std::string CircularBuffer::read(const size_t len) {
    // When there is nothing to pop, just return 0.
    if (empty() || len == 0) {
        return "";
    }

    // So now the buf_ is not empty. Popping will make it not full.
    full_ = false;

    std::string read_string;
    if (head_ > tail_) {
        if (len > head_ - tail_) {
            read_string = buf_.substr(tail_, head_ - tail_);
            tail_ = head_;
        } else {
            read_string = buf_.substr(tail_, len);
            tail_ += len;
        }
    } else {
        size_t tail_size = buf_.size() - tail_;
        if (len >= tail_size) {
            if (len - tail_size >= head_) {
                read_string = buf_.substr(tail_, tail_size) + buf_.substr(0, head_);
                tail_ = head_;
            } else {
                read_string = buf_.substr(tail_, tail_size) + buf_.substr(0, len - tail_size);
                tail_ = len - tail_size;
            }
        } else {
            read_string = buf_.substr(tail_, len);
            tail_ += len;
        }
    }
    return read_string;
}


size_t CircularBuffer::capacity() const { return max_size_; }

size_t CircularBuffer::used_size() const {
    if (full_) {
        return max_size_;
    }
    if (head_ >= tail_) {
        return head_ - tail_;
    } else {
        return max_size_ - tail_ + head_;
    }
}

bool CircularBuffer::empty() const { return head_ == tail_ && !full_; }

bool CircularBuffer::full() const { return full_; }

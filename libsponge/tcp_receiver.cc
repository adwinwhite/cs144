#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    //Check whether syn is sent
    if (seg.header().syn) {
        isn_ = seg.header().seqno;
        syn_received_ = true;
    }
    if (!syn_received_) {
        return;
    }
    if (seg.header().fin) {
        fin_received_ = true;
    }

    //Now that we have isn, push data to buffer.
    uint64_t index = unwrap(seg.header().seqno, isn_, reassembler_.stream_out().bytes_written());
    if (!seg.header().syn) {
        --index;
    }
    reassembler_.push_substring(string(seg.payload()), index, seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    //Add 1 when fin is received and all bytes are written
    if (syn_received_) {
        return wrap(reassembler_.stream_out().bytes_written() + 1 + (reassembler_.stream_out().input_ended() ? 1 : 0), isn_);
    } else {
        return {};
    }
}


size_t TCPReceiver::window_size() const {
    return reassembler_.stream_out().remaining_capacity();
}

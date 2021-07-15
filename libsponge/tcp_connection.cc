#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

using namespace std;

void TCPConnection::connect() {
    sender_.fill_window();
    send_segments();
    active_ = true;
}

void TCPConnection::send_segments() {
    while (sender_.segments_out().size() > 0) {
        TCPSegment seg = sender_.segments_out().front();
        if (receiver_.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = receiver_.ackno().value();
            seg.header().win = receiver_.window_size();
        }
        segments_out_.push(seg);
        sender_.segments_out().pop();
    }
}


void TCPConnection::unclean_shutdown() {
    TCPHeader header{};
    header.rst = true;
    sender_.send_empty_segment(header);
    sender_.stream_in().set_error();
    receiver_.stream_out().set_error();
    active_ = false;
}


size_t TCPConnection::write(const string &data) {
    const size_t written_len = sender_.stream_in().write(data);
    if (written_len > 0) {
        sender_.fill_window();
    }
    send_segments();
    return written_len;
}

size_t TCPConnection::remaining_outbound_capacity() const { return sender_.stream_in().remaining_capacity(); }

void TCPConnection::end_input_stream() {
    sender_.stream_in().end_input();
    TCPHeader header{};
    header.fin = true;
    sender_.send_empty_segment(header);
    send_segments();
}

size_t TCPConnection::bytes_in_flight() const { return sender_.bytes_in_flight(); }


size_t TCPConnection::unassembled_bytes() const { return receiver_.unassembled_bytes(); }


void TCPConnection::segment_received(const TCPSegment &seg) {
    time_since_last_segment_received_ = 0;
    //Check whether RST is set
    if (seg.header().rst) {
        sender_.stream_in().set_error();
        receiver_.stream_out().set_error();
        active_ = false;
        return;
    }

    //Notify receiver_
    receiver_.segment_received(seg);

    //Notify sender_
    if (seg.header().ack) {
        sender_.ack_received(seg.header().ackno, seg.header().win);
    }

    //Reply
    if (seg.length_in_sequence_space() > 0) {
        sender_.fill_window();
    }
    send_segments();
}

bool TCPConnection::active() const { return active_; }


//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    time_since_last_segment_received_ += ms_since_last_tick;
    if (linger_after_streams_finish_) {
        if (time_since_last_segment_received_ >= cfg_.rt_timeout * 10) {
            clean_shutdown();
        }
    }
    sender_.tick(ms_since_last_tick);
    if (sender_.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        unclean_shutdown();
    }
    send_segments();
}

void TCPConnection::clean_shutdown() {
    active_ = false;
}

size_t TCPConnection::time_since_last_segment_received() const { return time_since_last_segment_received_; }



TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            unclean_shutdown();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

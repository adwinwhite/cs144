#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

using namespace std;

void TCPConnection::connect() {
    sender_.fill_window();
    send_segments();
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
    //No need to send FIN when input ended.
    // TCPHeader header{};
    // header.fin = true;
    // sender_.send_empty_segment(header);
    sender_.fill_window();
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
        return;
    }

    //Notify receiver_
    receiver_.segment_received(seg);

    //If syn is not received, ignore the segment.
    if (!receiver_.syn_received()) {
        return;
    }



    //Passive close
    if (seg.header().fin && (!sender_.fin_sent())) {
        linger_after_streams_finish_ = false;
    }


    //Notify sender_
    if (seg.header().ack) {
        sender_.ack_received(seg.header().ackno, seg.header().win);
    }

    //Reply
    //If fill_window sends nothing, then we send an empty segment.
    if (!sender_.fill_window()) {
        if (seg.length_in_sequence_space() > 0) {
            sender_.send_empty_segment(TCPHeader{});
        }
    }
    send_segments();

    if (sender_.stream_in().eof() && sender_.next_seqno_absolute() == sender_.stream_in().bytes_written() + 2 && sender_.fin_sent() && sender_.bytes_in_flight() == 0 &&
            receiver_.stream_out().input_ended() && receiver_.fin_received()) {
        linger_started_ = true;
    }
}

bool TCPConnection::active() const {
    if (sender_.stream_in().error() || receiver_.stream_out().error()) {
        return false;
    }
    // Active since created.
    // if (sender_.next_seqno_absolute() == 0 && (!receiver_.ackno().has_value())) {
        // return false;
    // }
    if (sender_.stream_in().eof() && sender_.next_seqno_absolute() == sender_.stream_in().bytes_written() + 2 && sender_.bytes_in_flight() == 0 &&
            receiver_.stream_out().input_ended()) {
        if ((linger_after_streams_finish_ && linger_done_) || (!linger_after_streams_finish_)) {
            return false;
        }
    }
    return true;
}


//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    time_since_last_segment_received_ += ms_since_last_tick;
    if (linger_started_ && linger_after_streams_finish_) {
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
    receiver_.stream_out().end_input();
    linger_done_ = true;
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

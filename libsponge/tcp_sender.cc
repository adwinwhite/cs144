#include "tcp_sender.hh"

#include "tcp_config.hh"
#include "wrapping_integers.hh"
#include "buffer.hh"

#include <random>
#include <cmath>
#include <algorithm>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;
//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : isn_(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , initial_retransmission_timeout_{retx_timeout}
    , stream_(capacity) {}

uint64_t TCPSender::bytes_in_flight() const {
    return next_seqno_absolute() - ackno_;
}

void TCPSender::fill_window() {
    //You may need to send multiple segments.
    //When you send new data, this function is called.
    //Read as many bytes as you can from the input stream.
    //Though its number should be smaller than window_size and tcp segment maxsize
    //IN: stream_, segments_out_, segments_record_
    //OUT: stream_, segments_out_, segments_record_
    //
    bool seg_sent = false;
    uint16_t remaining_window_size = window_size_ == 0 ? 1 : (window_size_ >= bytes_in_flight() ? window_size_ - bytes_in_flight() : window_size_);
    while (remaining_window_size > 0) {
        const size_t data_len = min(static_cast<size_t>(remaining_window_size), TCPConfig::MAX_PAYLOAD_SIZE);


        string payload = stream_.read(data_len);

        //Construct tcp segment
        TCPSegment seg{};

        seg.header().seqno = next_seqno();

        //Check state
        //Check whether it is the first segment
        if (next_seqno_absolute() == 0) {
            //If eofed now, should I trigger error?
            seg.header().syn = true;
            state_ = TCPSenderState::SYN_SENT;
        }
        if (stream_.eof() && next_seqno_absolute() < stream_.bytes_written() + 2) {
            //If there is no room for FIN in window, do not add FIN.
            if (bytes_in_flight() + payload.size() < window_size_) {
                seg.header().fin = true;
                state_ = TCPSenderState::FIN_SENT;
            }
        }

        //Update next_seqno_
        if (state_ == TCPSenderState::SYN_SENT || state_ == TCPSenderState::SYN_ACKED) {
            next_seqno_ = stream_.bytes_read() + 1;
        } else if (state_ == TCPSenderState::FIN_SENT || state_ == TCPSenderState::FIN_ACKED) {
            next_seqno_ = stream_.bytes_read() + 2;
        } else {
            next_seqno_ = stream_.bytes_read();
        }


        //Send tcp segment
        //IN: header, payload, segments_out_, segments_record_
        //OUT: segments_out_, segments_record_
        seg.payload() = Buffer(std::move(payload));
        //If the segment occupies no segno, no need to send or save anything
        if (seg.length_in_sequence_space() != 0) {
            segments_out_.push(seg);
            segments_record_.push(seg);
            seg_sent = true;
        } else {
            break;
        }
        remaining_window_size = window_size_ - bytes_in_flight();
    }

    //Trigger timer
    if (seg_sent && !timer_.running()) {
        timer_.start(initial_retransmission_timeout_);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    const uint64_t new_ackno = unwrap(ackno, isn_, ackno_);
    window_size_ = window_size;

    if (new_ackno <= ackno_) {
        return;
    }
    //Now it says new data are received

    //Remove segments that's been acked
    while (segments_record_.size() > 0 && unwrap(segments_record_.front().header().seqno, isn_, ackno_) + segments_record_.front().length_in_sequence_space() <= new_ackno) {
        segments_record_.pop();
    }

    //Reset RTO
    consecutive_retransmissions_ = 0;
    //Stop timer if all outstanding segments are acked
    if (segments_record_.size() == 0) {
        timer_.reset();
    } else {
        timer_.start(initial_retransmission_timeout_);
    }

    ackno_ = new_ackno;

}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (timer_.running()) {
        timer_.tick(ms_since_last_tick);
    }
    if (timer_.expired()) {
        segments_out_.push(segments_record_.front());
        if (window_size_ > 0) {
            ++consecutive_retransmissions_;
        }
        timer_.start(initial_retransmission_timeout_ * pow(2, consecutive_retransmissions_));
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return consecutive_retransmissions_; }

void TCPSender::send_empty_segment() {
    TCPHeader header{};
    header.seqno = next_seqno();
    TCPSegment seg{};
    seg.parse(Buffer(header.serialize()));
    segments_out_.push(seg);
}

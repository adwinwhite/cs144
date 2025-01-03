#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.


using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    // Since uint is a group under addition
    uint32_t result = static_cast<uint32_t>(n & 0xffffffff) + isn.raw_value();
    return WrappingInt32{result};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t result = static_cast<uint64_t>(n.raw_value() - isn.raw_value());
    uint64_t high_part = checkpoint & 0xffffffff00000000;
    result += high_part;
    //Damn there are two nearest numbers to checkpoint possibly
    if (result > checkpoint + (1ul << 31)) {
        if (high_part != 0) {
            result -= (1ul << 32);
        }
    } else if (checkpoint >= (1ul << 31) && result < checkpoint - (1ul << 31)) {
        result += (1ul << 32);
    }
    return result;
}

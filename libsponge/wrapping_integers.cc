#include "wrapping_integers.hh"

#include <iostream>

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template<typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

static const auto round32 = 4294967296; // 1<<32 , start a new round , the max32bit is 4294967295

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return isn + (n % round32);
//    DUMMY_CODE(n, isn);
//    return WrappingInt32{0};
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

    // n - isn , 获得相对于0的值
    // 找到checkpoint所在的圈, 和上一个圈 , 比对和checkpoint的差值
    uint64_t absSeq = n.raw_value() - isn.raw_value();
    uint64_t locateRound = static_cast<int>(checkpoint / round32);

    uint64_t locateVal = locateRound*round32 + absSeq;

    uint64_t right,left;
    if (locateVal < checkpoint) {
        locateVal+=round32;
    }

    right = locateVal-checkpoint;
    left = checkpoint-(locateVal-round32);

    // another impl
    //    if (locateVal<round32) {
    //        left = checkpoint+round32 - (locateVal +2*round32 -round32);
    //    } else {
    //        left = checkpoint-(locateVal-round32);
    //    }

    if (locateVal>=round32) {
        if (right > left) {
            return locateVal - round32;
        }
    }
    return static_cast<uint64_t>(locateVal);


    /*
     * pesudo code
     * abs = n-isn
     * nearestRound = checkpoint  % round32;
     * left  = nearestRound -1;
     * right = nearestRound +1;
     * absGap1 = nearestRound * (round32)+abs;
     * absGap2 = left * (round32) +abs
     */


}

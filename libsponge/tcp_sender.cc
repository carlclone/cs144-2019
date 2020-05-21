#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

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
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , consecutiveCount(0)
    , hisWindowSize(1)
    , unAckWindowLeft(0)
    , unAckWindowRight(0)  //无符号数,得把区间定义改成前闭后开了
    , syncSent(false)
    , finSent(false)
    , retxQueue() {}

uint64_t TCPSender::bytes_in_flight() const { return unAckWindowRight - unAckWindowLeft; }

void TCPSender::fill_window() {
    if (!syncSent) {
        TCPSegment seg;
        seg.header().syn = true;
        seg.header().seqno = next_seqno();
        _next_seqno++;
        segments_out().push(seg);
        syncSent = true;
        /*
         * 维护窗口
         */
        unAckWindowRight++;
        /*
         * 重传相关实现
         */
        // retxQueue.push(seg);
    }

    auto remainWindowSize = hisWindowSize - bytes_in_flight();
    if (stream_in().buffer_size() > 0 && remainWindowSize > 0) {
        auto minSize = min(stream_in().buffer_size(), remainWindowSize);
        TCPSegment seg;
        seg.header().seqno = next_seqno();
        /*
         * 关闭
         */
        if (stream_in().input_ended()) {
            seg.header().fin = true;
            _next_seqno++;
        }

        /*
         * payload
         */

        Buffer buf{stream_in().peek_output(minSize)};
        stream_in().pop_output(minSize);

        seg.payload() = buf;
        _next_seqno += minSize;

        segments_out().push(seg);

        /*
         * 维护窗口
         */
        unAckWindowRight += minSize;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
//! \returns `false` if the ackno appears invalid (acknowledges something the TCPSender hasn't sent yet)
bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    auto absAck = unwrap(ackno, _isn, next_seqno_absolute());

    if (absAck < next_seqno_absolute()) {
        return true;
    }

    /*
     * 维护未 ack 窗口
     */
    unAckWindowLeft = min(unAckWindowRight, absAck);
    hisWindowSize = window_size;
    /*
     * 重传相关
     */
    // do sth...

    if (absAck > next_seqno_absolute()) {
        return false;
    } else {
        return true;
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); }

unsigned int TCPSender::consecutive_retransmissions() const { return consecutiveCount; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    segments_out().push(seg);
}

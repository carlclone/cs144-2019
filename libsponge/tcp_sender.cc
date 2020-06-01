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
    , unAckWindowRight(0)
    , syncSent(false)
    , finSent(false)
    , retxQueue()
    , retxTimeout(_initial_retransmission_timeout)
    , retxTimePass(0) {}

uint64_t TCPSender::bytes_in_flight() const { return unAckWindowRight - unAckWindowLeft; }

void TCPSender::fill_window() {
    if (finSent) {
        return;
    }

    TCPSegment seg;
    if (next_seqno_absolute() == 0) {
        seg.header().syn = true;
        seg.header().seqno = next_seqno();
        _next_seqno++;

        unAckWindowRight++;

        if (seg.length_in_sequence_space()) {
            segments_out().push(seg);
            retxQueue.push(seg);
        }
    } else {
        seg.header().seqno = next_seqno();

        auto remainWindowSize = hisWindowSize - bytes_in_flight();

        if (stream_in().input_ended() && remainWindowSize>0 ) {
            finSent = true;

            seg.header().fin = true;
            _next_seqno++;
            unAckWindowRight++;
            remainWindowSize--;
        }


        if (stream_in().buffer_size() > 0 && remainWindowSize > 0) {
            auto minSize = min(stream_in().buffer_size(), remainWindowSize);
            minSize = min(minSize,TCPConfig::MAX_PAYLOAD_SIZE);

            Buffer buf{stream_in().peek_output(minSize)};
            stream_in().pop_output(minSize);

            seg.payload() = buf;
            _next_seqno += minSize;

            remainWindowSize-=minSize;
            unAckWindowRight += minSize;
        }

        if (seg.length_in_sequence_space()) {
            segments_out().push(seg);
            retxQueue.push(seg);
        }

        while (stream_in().buffer_size() > 0 && remainWindowSize > 0) {
            TCPSegment tmpSeg;
            tmpSeg.header().seqno = next_seqno();
            auto minSize = min(stream_in().buffer_size(), remainWindowSize);
            minSize = min(minSize,TCPConfig::MAX_PAYLOAD_SIZE);
            Buffer buf{stream_in().peek_output(minSize)};
            stream_in().pop_output(minSize);

            tmpSeg.payload() = buf;
            _next_seqno += minSize;

            remainWindowSize-=minSize;
            unAckWindowRight += minSize;
            if (tmpSeg.length_in_sequence_space()) {
                segments_out().push(tmpSeg);
                retxQueue.push(tmpSeg);
            }
        }


    }




}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
//! \returns `false` if the ackno appears invalid (acknowledges something the TCPSender hasn't sent yet)
bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    auto absAck = unwrap(ackno, _isn, next_seqno_absolute());

    hisWindowSize = window_size;

    while (!retxQueue.empty()) {
        auto retxSeg = retxQueue.front();
        auto expectAbsAckno = unwrap(retxSeg.header().seqno, _isn, next_seqno_absolute()) + retxSeg.length_in_sequence_space();
        if (expectAbsAckno <= absAck) {
            retxQueue.pop();
            retxTimeout =  _initial_retransmission_timeout;
            retxTimePass=0;
            consecutiveCount=0;
        } else {
            break;
        }
    }
    if (absAck < next_seqno_absolute()) {
        return true;
    }




    unAckWindowLeft = min(unAckWindowRight, absAck);



    if (absAck > next_seqno_absolute()) {
        return false;
    } else {
        return true;
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    retxTimePass += ms_since_last_tick;

    if (retxTimePass >=retxTimeout) {
        if (!retxQueue.empty()) {
            consecutiveCount++;
            TCPSegment seg = retxQueue.front();
            segments_out().push(seg);
            retxTimeout*=2;
        }

        retxTimePass = 0;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return consecutiveCount; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    segments_out().push(seg);
}

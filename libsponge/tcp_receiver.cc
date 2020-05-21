#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {
    auto header = seg.header();
    auto data = seg.payload();
    if (header.syn && synced) {
        return false;
    }
    if (header.syn && !synced) {
        hisIsn = header.seqno;
        nextSeqno= unwrap(hisIsn+1,hisIsn,hisIsn.raw_value());
        synced=true;
    }
    if (header.fin && fined) {
        return false;
    }
    if (header.fin && !fined) {
        nextSeqno+=1;
        fined=true;
        stream_out().end_input();
    }

    if (synced) {
        auto absSeq = unwrap(header.seqno,hisIsn,nextSeqno);

        if (absSeq < nextSeqno) {
            if (absSeq + data.size() >= nextSeqno) {
                //may be need substr
                _reassembler.push_substring();
            } else {
                //out of window
                return false;
            }
        }

        if (absSeq-nextSeqno+1 > stream_out().remaining_capacity()) {
            //out of window
            return false;
        }

        //不是连续的,但在 窗口里,fit in的部分写入
        if (absSeq!=nextSeqno) {
            auto sub = data.str().substr(0,window_size());
        }

        //如果是连续的,则放入reassembler
        if (absSeq == nextSeqno) {
            _reassembler.push_substring();
            nextSeqno+=data.size();
        }
        return true;





    } else {
        return false;
    }

}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (synced) {
        //return hisIsn + stream_out().bytes_written()+1;
        return wrap(nextSeqno,hisIsn);
    }
    return optional<WrappingInt32>{};
}

size_t TCPReceiver::window_size() const {
    //cap - bytesinstream
    return stream_out().remaining_capacity();
}

#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {
    auto header = seg.header();
    if (header.syn && synced) {
        return false;
    }
    if (header.syn && !synced) {
        hisIsn = header.seqno;
        nextSeqno= hisIsn.raw_value() + 1;
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

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

    /*
     * 如果已经同步过了,拒绝该 syn
     */
    if (header.syn && synced) {
        return false;
    }

    /*
     * 首次 syn
     */
    if (header.syn && !synced) {
        hisIsn = header.seqno;
        nextSeqno= unwrap(hisIsn+1,hisIsn,hisIsn.raw_value());
        synced=true;
    }

    /*
     * 已经 fin 过,拒绝
     */
    if (header.fin && fined) {
        return false;
    }

    /*
     * 首次 fin
     */
    if (header.fin && !fined) {
        nextSeqno+=1;
        fined=true;
        stream_out().end_input();
    }

    /*
     * 将在窗口内的字节放入 reassembler 里
     */
    if (synced) {

        auto absSeqLeft = unwrap(header.seqno,hisIsn,nextSeqno);
        auto absSeqRight = absSeqLeft+data.size()-1;

        auto absWindowLeft = 123;
        auto absWindowRight=123;

        auto absMaxLeft = max(absSeqLeft,absWindowLeft);
        auto absMinRight = min(absSeqRight,absWindowRight);

        /*
         * calculate by absMax ,min
         */
        auto bytesInWindow = 123;

        _reassembler.push_substring(bytesInWindow,absMaxLeft,false);

        /*
         * 更新 window
         */




        if (absSeqLeft < nextSeqno) {
            /*
             * 小于 nextSeqno 但是有多的字节(or 在窗口内的字节),增量 ,
             * 截取在窗口内的字节,如果不为空则写入 reassembler
             */
            if (absSeqLeft + data.size() >= nextSeqno) {
                //may be need substr
                _reassembler.push_substring(data.str().data(), absSeqLeft, false);
            }

        }

        if (absSeqLeft > nextSeqno) {
            /*
             * 大于 seqno ,截取处于窗口内的字节,不为空则写入
             */
            auto sub = data.str().substr(0,window_size());
            _reassembler.push_substring(sub.data(), absSeqLeft,false);
        }

        /*
         * 截取在窗口内的字节,不为空写入
         */
        if (absSeqLeft == nextSeqno) {
            _reassembler.push_substring(data.str().data(), absSeqLeft, false);
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

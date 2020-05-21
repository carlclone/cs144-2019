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
        raw = nextSeqno;
        synced=true;
    }

    /*
     * 已经 fin 过,拒绝
     */
    if (header.fin && fined) {
        return false;
    }

    bool closeInput = false;
    /*
     * 首次 fin
     */
    if (header.fin && !fined) {
        fined=true;
        closeInput = true;
    }

    if (data.size()==0) {
        if (closeInput) {
            stream_out().end_input();
            nextSeqno+=1;
        }
        return true;
    }

    /*
     * 将在窗口内的字节放入 reassembler 里
     */
    if (synced) {

        auto absSeqLeft = unwrap(header.seqno,hisIsn,nextSeqno); //116
        auto absSeqRight = absSeqLeft+data.size()-1; //117

        auto absWindowLeft = nextSeqno; //116
        auto absWindowRight=nextSeqno+window_size()-1; //130

        auto absMaxLeft = max(absSeqLeft,absWindowLeft); // 116
        auto absMinRight = min(absSeqRight,absWindowRight); //117

        /*
         * calculate by absMax ,min
         */
        auto skipBytesInLeft =absMaxLeft - absSeqLeft; //0
        auto skipBytesInRight = absSeqRight - absMinRight; //0


        //0 , 1-0
        auto bytesInWindow = data.str().substr(skipBytesInLeft,data.size()-skipBytesInRight);

        if (bytesInWindow.size()==0) {
            return false;
        }

        // a , 116 ,
        _reassembler.push_substring(static_cast<std::string>(bytesInWindow),absMaxLeft-raw,false);

        /*
         * 更新 window/+
         */
        nextSeqno +=bytesInWindow.size();

        if (closeInput) {
            stream_out().end_input();
            nextSeqno+=1;
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

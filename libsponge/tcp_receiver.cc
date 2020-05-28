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
        //1
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
            nextSeqno++;
        }

        return true;
    }

    /*
     * 将在窗口内的字节放入 reassembler 里
     */
    if (synced) {

        //1       1
        auto absSeqLeft = unwrap(header.seqno,hisIsn,nextSeqno); //116
        //1+3=4   4
        auto absSeqRight = absSeqLeft+data.size()-1; //117

        //1  5
        auto absWindowLeft = nextSeqno; //116

        //1+4000=4001   max
        auto absWindowRight=nextSeqno+window_size()-1; //130

        //1   5
        auto absMaxLeft = max(absSeqLeft,absWindowLeft); // 116
        //4   4
        auto absMinRight = min(absSeqRight,absWindowRight); //117

        /*
         * calculate by absMax ,min
         */
        //0  4
        auto skipBytesInLeft =absMaxLeft - absSeqLeft; //0
        //4-4=0
        auto skipBytesInRight = absSeqRight - absMinRight; //0

        if (skipBytesInLeft+skipBytesInRight>=data.size()) {
            return false;
        }

        //0 , 1-0
        auto bytesInWindow = data.str().substr(skipBytesInLeft,data.size()-skipBytesInRight);


        auto beforeWritten = stream_out().bytes_written();
        // a , 116 ,
        _reassembler.push_substring(static_cast<std::string>(bytesInWindow),absMaxLeft-raw,false);

        /*
         * 更新 window/+
         */
        nextSeqno += (stream_out().bytes_written() - beforeWritten ); //5

        if (closeInput) {
            stream_out().end_input();
            nextSeqno++;
        }

        return true;
    } else {
        return false;
    }

}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (synced) {
        return wrap(nextSeqno,hisIsn);
    }
    return optional<WrappingInt32>{};
}

/*
 * hat window size should I send if theTCPReceiverwants to advertise a window sizethat’s bigger than will fit in theTCPSegment::header().winfield?Send the biggest value you can.  You might find thestd::numericlimitsclass helpful.
 */
size_t TCPReceiver::window_size() const {
    //cap - bytesinstream
    return stream_out().remaining_capacity();
}

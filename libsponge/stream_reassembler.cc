#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity),nextIndex(0),bytesAssembled(0),lastProvedSegment() {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    /*思路: 如果和当前的index吻合,则放入bytestream中
     *  如果不吻合,放到一个未重组的list中,或者map中 , 等待在它前面的index到达后 , 再取出来放入bytestream
     *  map的大小就是capacity , 最多只能有capacity个字节未重组
     * */


    if (index == nextIndex) {
        _output.write(data);
        auto count = data.size();
        bytesAssembled+=count;
        lastProvedSegment = data;
        nextIndex+=count;

        if (eof) {
            _output.end_input();
        }
    }

    if (index<nextIndex) {
        if (index+data.size()>nextIndex ) {
            bool match=true;
            for (size_t i=0;i<lastProvedSegment.size();i++) {
                if (data[i]!=lastProvedSegment[i]) {
                    match=false;
                    break;
                }
            }
            if (match) {
                auto subStr = data.substr(lastProvedSegment.size(),data.size()-lastProvedSegment.size());
                push_substring(subStr,nextIndex,eof);
            }
        }
        return;
    }




}

size_t StreamReassembler::unassembled_bytes() const {

    return 0;
//    return unassembytes;
}

bool StreamReassembler::empty() const {
    return false;
//    return un ==0;
}

#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity),nextIndex(0),bytesAssembled(0),bytesUnAssembled(0),lastProvedSegment(),tmpMap() {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {



    if (index<nextIndex) {
        /*
         * increment update
         */
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
        /*
         * immediately ignore
         */
        else {
            return;
        }
        return;
    }

    /*put every byte into map ,
     * if rech end and eof==true , set eof
     * if byte already in , ignore
     * */
    if (index > nextIndex) {
        for (size_t i=0;i<data.size();i++) {
            if (tmpMap.count(index+i)!=0) {
                continue;
            }
            if (i!=data.size()-1) {
                tmpMap[index+i] = pair<std::string,bool>(data.substr(i,1),false);
            } else {
                tmpMap[index+i] = pair<std::string,bool>(data.substr(i,1),eof);
            }

            bytesUnAssembled++;
        }
        return;
    }

    /*
     * put every byte into stream
     * if byte is in map, del it
     */
    if (index == nextIndex) {
        lastProvedSegment=data;
        for (size_t i=0;i<data.size();i++) {
            _output.write(data.substr(i,1));

            nextIndex++;

            if (tmpMap.count(index+i)!=0) {
                tmpMap.erase(index+i);
                bytesUnAssembled--;
            }
        }

        if (eof) {
            _output.end_input();
        }
    }

    //see if next byte is in map;
    if (tmpMap.count(nextIndex)!=0) {
        pair<std::string,bool> pair = tmpMap[nextIndex];
        tmpMap.erase(nextIndex);
        bytesUnAssembled--;
        push_substring(pair.first,nextIndex,pair.second);
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    return bytesUnAssembled;
}

bool StreamReassembler::empty() const {
    return false;
//    return un ==0;
}

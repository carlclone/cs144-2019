#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <stdexcept>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template<typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

/* CS110课程有相似的Lab
 * 字节从input写入 , 从output读出 ,
 * 容量有限,需要判断防溢出
 * writer可以end input , 和tcp的SHUT_WR一样 , 此时reader读到字节流尾部的时候回收到eof信号
 * reader读出数据,容量变大 , writer也要注意,溢出了要报错
 * 思路:使用循环队列
 *
 * */

ByteStream::ByteStream(const size_t capacity) :capcity(capacity),bytesInPipe(0),bytesReadTotal(0),bytesWriteTotal(0),_ended(false),bytesPipe() {}

/*把string转成字节数组,写入stream的管道里
 * 如果满了,报错
 *  如果写入完毕了,设置ended
 *
 * */
size_t ByteStream::write(const string &data) {
    //尽量写入,只有管道里还有空间
    size_t i;
    size_t minSize = min(data.size(),remaining_capacity());
    for ( i=0; i<minSize;i++) {
        bytesPipe.push_back(data[i]);
    }
    bytesWriteTotal+=minSize;
    bytesInPipe+=minSize;
    return minSize;
}

/*
 *           capcity-len      capcity
 * [       --------------------------]
 *
 */
//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    //返回管道中 0到0+len的字节 , 转成字符串
    size_t minSize = min(len,buffer_size());
    return string(bytesPipe.begin(),bytesPipe.begin()+minSize);
}

//同上
//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t minSize = min(len,buffer_size());
    size_t i;
    for (i = 0; i < minSize; i++) {
        bytesPipe.pop_front();
    }
    bytesInPipe-=minSize;
    bytesReadTotal+=minSize;

}


//`true` if the output has reached the ending
bool ByteStream::eof() const {
    if (bytesInPipe == 0 && input_ended()) {
        return true;
    }
    return false;
}
size_t ByteStream::bytes_written() const { return bytesWriteTotal; }
size_t ByteStream::bytes_read() const { return bytesReadTotal; }
// capcity-buffersize
size_t ByteStream::remaining_capacity() const { return capcity - buffer_size(); }
//要记录当前字节长度
size_t ByteStream::buffer_size() const { return bytesInPipe; }
//字节长度==0
bool ByteStream::buffer_empty() const { return buffer_size() == 0; }
//Signal that the byte stream has reached its ending
void ByteStream::end_input() { _ended = true; }
//`true` if the stream input has ended
bool ByteStream::input_ended() const { return _ended; }
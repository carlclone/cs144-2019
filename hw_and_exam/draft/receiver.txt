transmit test

test1
//回应syn , 保存对方的seqno 
//处理带data的数据和seqno状态
//生成正确的ackno
//将数据放入reassembler

test2

//ack只有在前面没有空洞的时候才会返回

//ack理解为,这个num前面的都收到了,下次从这个开始发

//seq num理解为,这次要发的数据放置在字节流中的这个num处


test3

x

test4


# test case complex level
connect
close
transmit
window
reorder

pesudo code

//ack理解为,这个num前面的都收到了,下次从这个开始发 (next expected data)
//seq num理解为,这次要发的数据放置在字节流中的这个num处
//syn和ack也占一位seqno

// the index of the last reassembled byte as the checkpoint.
//注意绝对seqno和相对seqno , 还有字节流的index要剔除掉syn和fin ,  别弄混了

len() = length_insequence_space()

property syned ,fined,  ackNo 
property capacity , reassembler , bytestream




if segement.syn && syned 
    return no ok;

if segment.syn && !syned
    hisIsn = segment.seqno
    nextSeqno = segment.seqno+1 //abs , not wrapping
  
    syned=true
    return ok

if segment.fin && fined 
    return no ok;


if segment.fin && !fined
    // do sth ...
    nextSeqno +=1
    fined=true
    stream_out.end_input()

if syned
    absSeq = unwrap(seqno,hisIsn,nextSeqno)
    if absSeq < nextSeqno
        
        if absSeq+segment.data.size()>=nextSeqno //这个可能要切割,可能超过window size  ( bs负责提供连续的字节流给上层 , reass负责重组 , receiver负责flow control , 保证不会超过容量 , 就由receiver做切割吧
            
            push_string(segement.data,absSeq)
        else 
            return out of window
        
    if absSeq-nextSeqno+1 > remaining_capacity
        return out of window    
        
    //不是连续的,但在 窗口里,fit in的部分写入
    if absSeq!=nextSeqno 
        sub = substr(segment.data,0, window_size() )
        reassembler.push_string(sub,absSeq)

    //如果是连续的,则放入reassembler
    
    if absSeq == nextSeqno
        reassembler.push_string(segment.data,absSeq)
        nextSeqno +=len(data)

        return [ack=wrap(nextSeqno) , ok]
       



//The lower (sometimes called “left”) edge of the window is called the ackno: it’s the index of
  the first byte that the TCPReceiver doesn’t already know
  
//it’s
  the sequence number of the first byte that the receiver hasn’t already been able to reassemble
  
//higher (or “right”) edge of the window is the first index that the TCPReceiver is not
  willing to accept
  
//In practice, your
  receiver will announce a window size equal to its capacity minus the number of bytes being
  held in its StreamReassembler’s ByteStream.


//receive window size
// lower = ackno-1 = last segment received
// higher = last acceptable segment

higher-lower <=rws
window = higher-lower


//
func window()
    cap - bytesinstream
    
    
    //暂时不实现cumulative task
fun ack()
    return hisIsn + stream_out().bytes_written()+1;
# Lab4 TCPConnection

2020/05/28 ~ 06/01

[guide](https://cs144.github.io/assignments/lab4.pdf)

[finite state machine 视频讲解](https://www.bilibili.com/video/BV137411Z7LR?p=28)

[TCP Connection](https://github.com/carlclone/CS144/blob/master/libsponge/tcp_connection.cc)

```

TCP协议的Lab告一段落,后面的是发送EthernetFrame和Router的lab

1. fsm是(finite state machine)的缩写,有限状态机

2. 重传列表的问题 , cpp的queue在空的时候也会返回一个默认值 , 需要判断空的情况 , 对语言的库不熟悉,排查了好久,之后都要当心为空返回默认值的情况 (虽然我觉得为空的时候peek front应该报错)

3. 主动关闭才需要linger , 被动关闭不需要 ,被动关闭的判断（inbound在outbound之前关闭)

4.每次tick timeout都需要重置, 但只有重传的时候才执行exponential backoff

5. 触发RST的场景

6.linger的判断

7.debug skill++ (ide断点, 还是没有打log好用)
```

### 关于连接关闭

由于["Two General's Problem'](https://en.wikipedia.org/wiki/Two_Generals%27_Problem) , clean shutdown是不可能实现的一个目标 , 但tcp使用了一个非常接近的实现 , 在送出最后一个ack之后等待10*rt_timeout,如果对端没有重传,则假设达成了clean shutdown

### 重要的图示

```
connection 和 receiver/sender对应的state

switch (state) {
        case TCPState::State::LISTEN:
            _receiver = TCPReceiverStateSummary::LISTEN;
            _sender = TCPSenderStateSummary::CLOSED;
            break;
        case TCPState::State::SYN_RCVD:
            _receiver = TCPReceiverStateSummary::SYN_RECV;
            _sender = TCPSenderStateSummary::SYN_SENT;
            break;
        case TCPState::State::SYN_SENT:
            _receiver = TCPReceiverStateSummary::LISTEN;
            _sender = TCPSenderStateSummary::SYN_SENT;
            break;
        case TCPState::State::ESTABLISHED:
            _receiver = TCPReceiverStateSummary::SYN_RECV;
            _sender = TCPSenderStateSummary::SYN_ACKED;
            break;
        case TCPState::State::CLOSE_WAIT:
            _receiver = TCPReceiverStateSummary::FIN_RECV;
            _sender = TCPSenderStateSummary::SYN_ACKED;
            _linger_after_streams_finish = false;
            break;
        case TCPState::State::LAST_ACK:
            _receiver = TCPReceiverStateSummary::FIN_RECV;
            _sender = TCPSenderStateSummary::FIN_SENT;
            _linger_after_streams_finish = false;
            break;
        case TCPState::State::CLOSING:
            _receiver = TCPReceiverStateSummary::FIN_RECV;
            _sender = TCPSenderStateSummary::FIN_SENT;
            break;
        case TCPState::State::FIN_WAIT_1:
            _receiver = TCPReceiverStateSummary::SYN_RECV;
            _sender = TCPSenderStateSummary::FIN_SENT;
            break;
        case TCPState::State::FIN_WAIT_2:
            _receiver = TCPReceiverStateSummary::SYN_RECV;
            _sender = TCPSenderStateSummary::FIN_ACKED;
            break;
        case TCPState::State::TIME_WAIT:
            _receiver = TCPReceiverStateSummary::FIN_RECV;
            _sender = TCPSenderStateSummary::FIN_ACKED;
            break;
        case TCPState::State::RESET:
            _receiver = TCPReceiverStateSummary::ERROR;
            _sender = TCPSenderStateSummary::ERROR;
            _linger_after_streams_finish = false;
            _active = false;
            break;
        case TCPState::State::CLOSED:
            _receiver = TCPReceiverStateSummary::FIN_RECV;
            _sender = TCPSenderStateSummary::FIN_ACKED;
            _linger_after_streams_finish = false;
            _active = false;
            break;
    }

```

> https://cs144.github.io/lab_faq.html

![](https://github.com/carlclone/CS144/blob/master/hw_and_exam/imgs/receiver-evolution.svg)

![](https://github.com/carlclone/CS144/blob/master/hw_and_exam/imgs/sender-evolution.svg)

> https://cs144.github.io/assignments/lab4.pdf

![](https://github.com/carlclone/CS144/blob/master/hw_and_exam/imgs/tcp-connection.jpg)

> https://www.bilibili.com/video/BV137411Z7LR?p=28

![](https://github.com/carlclone/CS144/blob/master/hw_and_exam/imgs/tcp-state.jpg)

> https://blog.csdn.net/wuji0447/article/details/78356875

![](https://github.com/carlclone/CS144/blob/master/hw_and_exam/imgs/close.jpg)


# Lab3 TCPSender

2020/05/24 ~ 05/27

[note/pesudo](https://github.com/carlclone/CS144/blob/master/hw_and_exam/draft/sender.txt)

[TCPSender](https://github.com/carlclone/CS144/blob/master/libsponge/tcp_sender.cc)

[guide](https://cs144.github.io/assignments/lab3.pdf)

```
1.超时机制是 exponential backoff , 指数补偿

2.下一个 lab 还是多写点 helper 吧

3.fin 也占一个窗口位

4.只要收到 ack 就要更新窗口

5.cpp 的 list 遍历,erase 后返回下一个 iterator 的指针 

6.segfault 要怎么调试?

7.前闭后开区间的意义: unsigned int 不能为负值,前闭后开可以定义大小为 0 的区间

8.初始 windowSize 为 1 (发 syn)
```



# Lab2 TCPReceiver

2020/05/20 ~ 05/23

[note/pesudo](https://github.com/carlclone/CS144/blob/master/hw_and_exam/draft/receiver.txt)

[TCPReceiver](https://github.com/carlclone/CS144/blob/master/libsponge/tcp_receiver.cc)

[guide](https://cs144.github.io/assignments/lab2.pdf)

[sliding window 视频讲解](https://www.bilibili.com/video/BV137411Z7LR?p=33)

```
1.实现对 syn,fin,和带data 的 tcp segment 的接收,处理和响应

2.看着 test case 写了第一版 pesudo code, 然后直接在 pesudo code 上重构了一版,以窗口为模型,截取在窗口内的字节流

3.最后精简完,就 100 多行代码...用了 2 天

4.测试用例很完善, 真的感谢课程作者

5.lab2 对 lab1的 reassembler 增加了容量限制的 test case , 又回去 debug 了一会 , 涉及容量的问题只取在窗口内的字节就行了

6.同时收到 fin 和数据的时候,要处理完数据再处理 fin

7.window size只更新连续字节流部分

8.还没找到这套测试框架打印数据的方法

9.写入 Reassembler 的时候要用absolute index, 从 0 开始的,不计算 syn 位的索引
```

# Lab1 StreamReassembler

2020/05/18 ~ 05/20

[notes/pesudo](https://github.com/carlclone/CS144/blob/master/hw_and_exam/draft/reassembler.txt)

[Reassembler](https://github.com/carlclone/CS144/blob/master/libsponge/stream_reassembler.cc)

[guide](https://cs144.github.io/assignments/lab1.pdf)

```
1.lab1用了5，6个小时的时间才做完，测试用例有点难度

2.实现的是`TCP`的一个组件，重新组装segment成为连续的字节流，提供给上层应用

3.本来想使用递归让代码看起来清楚一点，结果`segfault`了，还是改成循环了

4.在虚拟机里写的代码，没有输入法，写的英文注释看起来挺美观，蹩脚的英文

5.没有人肉走完`test case`就开始写代码，结果是想的模型不对，不能满足后面的`case`，基本是推翻重写 （第一次是用`segment`，字符串的角度进行重组，但遇到了增量更新，未重组字节级别的覆盖，重组字节级别覆盖的`case`，最后把思考的模型改成字节流，代码也清晰许多）,这点让我想到业务开发里也是一样，没有往后思考可能的需求（更完善的`test case` ） ，导致代码只能东拼西凑满足功能，完全不堪入目。

7.pesudo code + test case is a powerful combo

8.maybe multiple eof , try store every eof state

9 implemented an out of order , repeat, overlapping , increment update , reliable reassembler
```


# Lab0 ByteStream

2020/05/17 ~ 05/18

[ByteStream](https://github.com/carlclone/CS144/blob/master/libsponge/byte_stream.cc)

[webget.cc](https://github.com/carlclone/CS144/blob/master/apps/webget.cc)

[guide](https://cs144.github.io/assignments/lab0.pdf)

```
0.lab0第一个任务是一个简单的网页内容获取程序 , 直接调用 Linux 的 TCP 协议栈接口 ,会在之后完成 lab4 时,使用自己写的 TCP 协议替换系统实现

1.第二个任务实现了一个in-memory reliable ordered bystream,做这个的目的是因为tcp实际上就是这样的一个可靠有序的字节流,不过是基于网络的,
实现这一个小lab可以更好地理解了tcp两端的一些交互 . 实际上是一个字节流的队列 , 和 Java 里的 stream 设计类似, 一端 writer 写入,
一端 reader 读出,写入一端有关闭操作 , 重点还是要理解 eof 的判断

比如说http的请求中,客户端发送完请求后会向服务端发送一个SHUTDOWN_WRITE的 信号, 就对应byteSteam类的input_ended(),
这时候服务端就知道,在这个sequence number之后没有新数据了,接收完前面的字节就可以开始处理请求, 这里的seq number就可以用作eof的判断

2.`webget`这个`task`用课程提供的虚拟镜像运行不了,折腾了好久,放到自己的`ubuntu16.04`里成功运行,越底层的语言开发环境越折磨人

```

#include "sender_harness.hh"
#include "tcp_config.hh"
#include "wrapping_integers.hh"

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

using namespace std;

int main() {

//    TCPSegment seg;
//    std::list<TCPSegment> l;
//    TCPSegment seg2;
//    seg.header().seqno=WrappingInt32(123);
//    l.push_front(seg);
//    l.push_front(seg2);
//
//    auto i = l.begin();
//    for (;i!=l.end();) {
//
//        std::cout << i->header().seqno;
//        i = l.erase(i);
//    }
//
//
//
//    exit(0);
    try {
        auto rd = get_random_generator();

        {
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;


            TCPSenderTestHarness test{"FIN sent test", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(AckReceived{WrappingInt32{isn + 1}});
            test.execute(ExpectState{TCPSenderStateSummary::SYN_ACKED});
            test.execute(Close{});
            test.execute(ExpectState{TCPSenderStateSummary::FIN_SENT});
            test.execute(ExpectBytesInFlight{1});
            test.execute(ExpectSegment{}.with_fin(true).with_seqno(isn + 1));
            test.execute(ExpectNoSegment{});
        }

        {
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;

            TCPSenderTestHarness test{"FIN acked test", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(AckReceived{WrappingInt32{isn + 1}});
            test.execute(ExpectState{TCPSenderStateSummary::SYN_ACKED});
            test.execute(Close{});
            test.execute(ExpectState{TCPSenderStateSummary::FIN_SENT});
            test.execute(ExpectSegment{}.with_fin(true).with_seqno(isn + 1));
            test.execute(AckReceived{WrappingInt32{isn + 2}});
            test.execute(ExpectState{TCPSenderStateSummary::FIN_ACKED});
            test.execute(ExpectBytesInFlight{0});
            test.execute(ExpectNoSegment{});
        }

        {//pass
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;

            TCPSenderTestHarness test{"FIN not acked test", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(AckReceived{WrappingInt32{isn + 1}});
            test.execute(ExpectState{TCPSenderStateSummary::SYN_ACKED});
            test.execute(Close{});
            test.execute(ExpectState{TCPSenderStateSummary::FIN_SENT});
            test.execute(ExpectSegment{}.with_fin(true).with_seqno(isn + 1));
            test.execute(AckReceived{WrappingInt32{isn + 1}}); //重复ack syn请求
            test.execute(ExpectState{TCPSenderStateSummary::FIN_SENT});
            test.execute(ExpectBytesInFlight{1}); // FIN还没被ACK, 所以1byte
            test.execute(ExpectNoSegment{});
        }

        {//pass 超时重传case
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;

            TCPSenderTestHarness test{"FIN retx test", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(AckReceived{WrappingInt32{isn + 1}});
            test.execute(ExpectState{TCPSenderStateSummary::SYN_ACKED});
            test.execute(Close{});
            test.execute(ExpectState{TCPSenderStateSummary::FIN_SENT});
            test.execute(ExpectSegment{}.with_fin(true).with_seqno(isn + 1));
            test.execute(AckReceived{WrappingInt32{isn + 1}});
            test.execute(ExpectState{TCPSenderStateSummary::FIN_SENT});
            test.execute(ExpectBytesInFlight{1});
            test.execute(ExpectNoSegment{});
            // 999毫秒后触发一次tick , 还没到1000毫秒,不会重传
            test.execute(Tick{TCPConfig::TIMEOUT_DFLT - 1});
            test.execute(ExpectState{TCPSenderStateSummary::FIN_SENT});
            test.execute(ExpectBytesInFlight{1});
            test.execute(ExpectNoSegment{});
            test.execute(Tick{1}); //1毫秒后触发一次tick , =1000ms , 触发重传
            test.execute(ExpectState{TCPSenderStateSummary::FIN_SENT});
            test.execute(ExpectBytesInFlight{1});
            //有一个待发送的segment在queue里
            test.execute(ExpectSegment{}.with_fin(true).with_seqno(isn + 1));
            test.execute(ExpectNoSegment{});
            test.execute(Tick{1});
            test.execute(ExpectState{TCPSenderStateSummary::FIN_SENT});
            test.execute(ExpectBytesInFlight{1});
            test.execute(ExpectNoSegment{});
            //收到fin的ack
            test.execute(AckReceived{WrappingInt32{isn + 2}});
            test.execute(ExpectState{TCPSenderStateSummary::FIN_ACKED});
            test.execute(ExpectBytesInFlight{0});
            test.execute(ExpectNoSegment{});
        }

    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }

    return EXIT_SUCCESS;
}

#include "sender_harness.hh"
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
    try {
        auto rd = get_random_generator();

        {
            //pass
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;

            TCPSenderTestHarness test{"Repeat ACK is ignored", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(ExpectNoSegment{});
            //对某个 segment 的 ack , 需要存储 segment,和正确 ack 的 ackno
            //有新的 ack 到的时候查表,如果存在则 true
            test.execute(AckReceived{WrappingInt32{isn + 1}});
            test.execute(WriteBytes{"a"});
            test.execute(ExpectSegment{}.with_no_flags().with_data("a"));
            test.execute(ExpectNoSegment{});
            test.execute(AckReceived{WrappingInt32{isn + 1}});
            test.execute(ExpectNoSegment{});
        }

        {
            //pass
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;

            TCPSenderTestHarness test{"Old ACK is ignored", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(ExpectNoSegment{});
            test.execute(AckReceived{WrappingInt32{isn + 1}});
            test.execute(WriteBytes{"a"});
            test.execute(ExpectSegment{}.with_no_flags().with_data("a"));
            test.execute(ExpectNoSegment{});
            test.execute(AckReceived{WrappingInt32{isn + 2}});
            test.execute(ExpectNoSegment{});
            test.execute(WriteBytes{"b"});
            test.execute(ExpectSegment{}.with_no_flags().with_data("b"));
            test.execute(ExpectNoSegment{});
            test.execute(AckReceived{WrappingInt32{isn + 1}});
            test.execute(ExpectNoSegment{});
        }

        {
            //pass
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;
            //
            TCPSenderTestHarness test{"Early ACK results in bare ACK", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(ExpectNoSegment{});
            test.execute(AckReceived{WrappingInt32{isn + 1}});
            test.execute(WriteBytes{"a"});
            test.execute(ExpectSegment{}.with_no_flags().with_data("a"));
            test.execute(ExpectNoSegment{});
            test.execute(AckReceived{WrappingInt32{isn + 17}});
            test.execute(ExpectSegment{}.with_seqno(isn + 2));
            test.execute(ExpectNoSegment{});
        }

    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }

    return EXIT_SUCCESS;
}

#include "receiver_harness.hh"
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
        // auto rd = get_random_generator();

        {
            //pass
            // Window size decreases appropriately
            size_t cap = 4000;
            uint32_t isn = 23452;
            TCPReceiverTestHarness test{cap};
            test.execute(SegmentArrives{}.with_syn().with_seqno(isn).
            with_result(SegmentArrives::Result::OK));
            test.execute(ExpectAckno{WrappingInt32{isn + 1}});
            test.execute(ExpectWindow{cap});
            test.execute(
                SegmentArrives{}.with_seqno(isn + 1).with_data("abcd").
                with_result(SegmentArrives::Result::OK));
            test.execute(ExpectAckno{WrappingInt32{isn + 5}});
            test.execute(ExpectWindow{cap - 4});

            //todo; 不连续的,fit in size ,切割后先放入reassembler里
            test.execute(
                SegmentArrives{}.with_seqno(isn + 9).with_data("ijkl").
                with_result(SegmentArrives::Result::OK));
            //ackno和window都还不变
            test.execute(ExpectAckno{WrappingInt32{isn + 5}});
            test.execute(ExpectWindow{cap - 4});

            //当连续的到达之后, 加上byteStream的bytes_written
            test.execute(
                SegmentArrives{}.with_seqno(isn + 5).with_data("efgh").
                with_result(SegmentArrives::Result::OK));

            test.execute(ExpectAckno{WrappingInt32{isn + 13}});
            test.execute(ExpectWindow{cap - 12});
        }

        {
            //passed
            // Window size expands upon read
            size_t cap = 4000;
            uint32_t isn = 23452;
            TCPReceiverTestHarness test{cap};
            test.execute(SegmentArrives{}.with_syn().with_seqno(isn).with_result(SegmentArrives::Result::OK));
            test.execute(ExpectAckno{WrappingInt32{isn + 1}});
            test.execute(ExpectWindow{cap});
            test.execute(
                SegmentArrives{}.with_seqno(isn + 1).with_data("abcd").with_result(SegmentArrives::Result::OK));
            test.execute(ExpectAckno{WrappingInt32{isn + 5}});
            test.execute(ExpectWindow{cap - 4});
            test.execute(ExpectBytes{"abcd"});
            test.execute(ExpectAckno{WrappingInt32{isn + 5}});
            test.execute(ExpectWindow{cap});
        }

        {
            // passed
            // high-seqno segment is rejected
            size_t cap = 2;
            uint32_t isn = 23452;
            TCPReceiverTestHarness test{cap};
            test.execute(SegmentArrives{}.with_syn().with_seqno(isn).with_result(SegmentArrives::Result::OK));
            test.execute(ExpectAckno{WrappingInt32{isn + 1}});
            test.execute(ExpectWindow{cap});
            test.execute(ExpectTotalAssembledBytes{0});

            test.execute(SegmentArrives{}.with_seqno(isn + 3).with_data("cd").with_result(
                SegmentArrives::Result::OUT_OF_WINDOW));
            test.execute(ExpectAckno{WrappingInt32{isn + 1}});
            test.execute(ExpectWindow{cap});
            test.execute(ExpectTotalAssembledBytes{0});
        }

        {
            // pass
            // almost-high-seqno segment is accepted, but only some bytes are kept
            size_t cap = 2;
            uint32_t isn = 23452;
            TCPReceiverTestHarness test{cap};
            test.execute(SegmentArrives{}.with_syn().with_seqno(isn).with_result(SegmentArrives::Result::OK));
            //只存下了b , 超出容量的部分去除 ,reassembler只重组容量内的字节 , (这个逻辑的职责是不是交给reassembler比较好 , 之前的case好像没覆盖到这情况)
            test.execute(SegmentArrives{}.with_seqno(isn + 2).with_data("bc").with_result(SegmentArrives::Result::OK));
            test.execute(ExpectTotalAssembledBytes{0});


            test.execute(SegmentArrives{}.with_seqno(isn + 1).with_data("a").with_result(SegmentArrives::Result::OK));
            test.execute(ExpectAckno{WrappingInt32{isn + 3}});
            test.execute(ExpectWindow{0});
            test.execute(ExpectTotalAssembledBytes{2});
            test.execute(ExpectBytes{"ab"});
            test.execute(ExpectWindow{2});
        }

        {
            //passed
            // low-seqno segment is rejected
            size_t cap = 4;
            uint32_t isn = 294058;
            TCPReceiverTestHarness test{cap};
            test.execute(SegmentArrives{}.with_syn().with_seqno(isn).with_result(SegmentArrives::Result::OK));
            test.execute(SegmentArrives{}.with_data("ab").with_seqno(isn + 1).with_result(SegmentArrives::Result::OK));
            test.execute(ExpectTotalAssembledBytes{2});
            test.execute(ExpectWindow{cap - 2});
            test.execute(SegmentArrives{}.with_data("ab").with_seqno(isn + 1).with_result(
                SegmentArrives::Result::OUT_OF_WINDOW));
            test.execute(ExpectTotalAssembledBytes{2});
        }

        {
            // 勉强pass , 有点tricky
            // almost-low-seqno segment is accepted
            size_t cap = 4;
            uint32_t isn = 294058;
            TCPReceiverTestHarness test{cap};
            test.execute(SegmentArrives{}.with_syn().with_seqno(isn).with_result(SegmentArrives::Result::OK));
            test.execute(SegmentArrives{}.with_data("ab").with_seqno(isn + 1).with_result(SegmentArrives::Result::OK));
            test.execute(ExpectTotalAssembledBytes{2});
            test.execute(ExpectWindow{cap - 2});
            //又是增量更新,
            test.execute(SegmentArrives{}.with_data("abc").with_seqno(isn + 1).with_result(SegmentArrives::Result::OK));
            test.execute(ExpectTotalAssembledBytes{3});
            test.execute(ExpectWindow{cap - 3});
        }

        { //pass
            // second SYN is rejected
            size_t cap = 2;
            uint32_t isn = 23452;
            TCPReceiverTestHarness test{cap};
            test.execute(SegmentArrives{}.with_syn().with_seqno(isn).with_result(SegmentArrives::Result::OK));
            test.execute(
                SegmentArrives{}.with_syn().with_seqno(isn).with_result(SegmentArrives::Result::OUT_OF_WINDOW));
            test.execute(ExpectWindow{cap});
            test.execute(ExpectTotalAssembledBytes{0});
        }

        {  // pass
            // second FIN is rejected
            size_t cap = 2;
            uint32_t isn = 23452;
            TCPReceiverTestHarness test{cap};
            test.execute(SegmentArrives{}.with_syn().with_seqno(isn).
            with_result(SegmentArrives::Result::OK));
            test.execute(SegmentArrives{}.with_fin().with_seqno(isn + 1).
            with_result(SegmentArrives::Result::OK));
            test.execute(
                SegmentArrives{}.with_fin().with_seqno(isn + 1).
                with_result(SegmentArrives::Result::OUT_OF_WINDOW));
            test.execute(ExpectWindow{cap});
            test.execute(ExpectTotalAssembledBytes{0});
        }

        {
            //pass
            // Segment overflowing the window on left side is acceptable.
            size_t cap = 4;
            uint32_t isn = 23452;
            TCPReceiverTestHarness test{cap};
            test.execute(SegmentArrives{}.with_syn().with_seqno(isn).with_result(SegmentArrives::Result::OK));
            test.execute(SegmentArrives{}.with_seqno(isn + 1).with_data("ab").with_result(SegmentArrives::Result::OK));
            test.execute(
                SegmentArrives{}.with_seqno(isn + 3).with_data("cdef").with_result(SegmentArrives::Result::OK));
        }

        {
            // Segment matching the window is acceptable.
            //pass
            size_t cap = 4;
            uint32_t isn = 23452;
            TCPReceiverTestHarness test{cap};
            test.execute(SegmentArrives{}.with_syn().with_seqno(isn).with_result(SegmentArrives::Result::OK));
            test.execute(SegmentArrives{}.with_seqno(isn + 1).with_data("ab").with_result(SegmentArrives::Result::OK));
            test.execute(SegmentArrives{}.with_seqno(isn + 3).with_data("cd").with_result(SegmentArrives::Result::OK));
        }

    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }

    return EXIT_SUCCESS;
}

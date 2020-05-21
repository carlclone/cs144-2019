#include "test_should_be.hh"
#include "wrapping_integers.hh"

#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

int main() {
  //  cout<<unwrap(WrappingInt32(UINT32_MAX), WrappingInt32(0), 0);
//    test_should_be(unwrap(WrappingInt32(UINT32_MAX), WrappingInt32(0), 0), static_cast<uint64_t>(UINT32_MAX));
//exit(0);
    try {
        // Unwrap the first byte after ISN
        test_should_be(unwrap(WrappingInt32(1), WrappingInt32(0), 0), 1ul);
        // Unwrap the first byte after the first wrap
        //just start a new round , find which round is it ,  find the most close to the cp's (round + wrapper)
        test_should_be(unwrap(WrappingInt32(1), WrappingInt32(0), UINT32_MAX), (1ul << 32) + 1);
        // Unwrap the last byte before the third wrap
        test_should_be(unwrap(WrappingInt32(UINT32_MAX - 1), WrappingInt32(0), 3 * (1ul << 32)), 3 * (1ul << 32) - 2);
        // Unwrap the 10th from last byte before the third wrap
        test_should_be(unwrap(WrappingInt32(UINT32_MAX - 10), WrappingInt32(0), 3 * (1ul << 32)), 3 * (1ul << 32) - 11);
        // Non-zero ISN
        //TODO;   sub isn , then same as above
        test_should_be(unwrap(WrappingInt32(UINT32_MAX), WrappingInt32(10), 3 * (1ul << 32)), 3 * (1ul << 32) - 11);
        // Big unwrap , your impl possible overflow ? seems not , just equal
        test_should_be(unwrap(WrappingInt32(UINT32_MAX), WrappingInt32(0), 0), static_cast<uint64_t>(UINT32_MAX));
        // Unwrap a non-zero ISN  x
        test_should_be(unwrap(WrappingInt32(16), WrappingInt32(16), 0), 0ul);

        // Big unwrap with non-zero ISN TODO; -1 round ? 一个环的模型,但是向下溢出要怎么做? 负数的话, 32bitlen+负数
        test_should_be(unwrap(WrappingInt32(15), WrappingInt32(16), 0), static_cast<uint64_t>(UINT32_MAX));
        // Big unwrap with non-zero ISN ,
        test_should_be(unwrap(WrappingInt32(0), WrappingInt32(INT32_MAX), 0), static_cast<uint64_t>(INT32_MAX) + 2);
        // Barely big unwrap with non-zero ISN
        test_should_be(unwrap(WrappingInt32(UINT32_MAX), WrappingInt32(INT32_MAX), 0), static_cast<uint64_t>(1) << 31);
        // Nearly big unwrap with non-zero ISN
        test_should_be(unwrap(WrappingInt32(UINT32_MAX), WrappingInt32(1ul << 31), 0),
                       static_cast<uint64_t>(UINT32_MAX) >> 1);
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }

    return EXIT_SUCCESS;
}

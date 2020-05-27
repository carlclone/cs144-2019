#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const {
    return _cfg.send_capacity - _sender.bytes_in_flight();
}

size_t TCPConnection::bytes_in_flight() const {
    return _sender.bytes_in_flight();
}

size_t TCPConnection::unassembled_bytes() const {
    return _receiver.unassembled_bytes();
}

size_t TCPConnection::time_since_last_segment_received() const {
    return timeSinceLastSegmentReceived;
}

void TCPConnection::segment_received(const TCPSegment &seg) {
//    timeSinceLastSegmentReceived=0;



    //This flag (“reset”) means instant death to the connection. If you receive a segment
    //with rst , you should set the error flag on the inbound and outbound ByteStreams,
    //and any subsequent call to TCPConnection::active() should return false.
    auto header= seg.header();
    auto payload=seg.serialize();
    if (expectingSyn && !header.syn) {
        return;
    } else {
        expectingSyn=false;
    }
    if (header.ack) {
        _sender.ack_received(header.ackno,header.win);
    }

    if (seg.length_in_sequence_space()==0) {
        return;
    }
    _receiver.segment_received(seg);

    TCPSegment ongoingSeg;
    if (_sender.segments_out().size()==0) {
        _sender.send_empty_segment();
    }
    ongoingSeg = _sender.segments_out().front();
    _sender.segments_out().pop();

    std::optional<WrappingInt32> ackno = _receiver.ackno();
    if (ackno.has_value()) {
        ongoingSeg.header().ack = true;
        ongoingSeg.header().ackno = ackno.value();
        ongoingSeg.header().win = _receiver.window_size();
    }

    segments_out().push(ongoingSeg);
}


bool TCPConnection::active() const {
    return true;
}

size_t TCPConnection::write(const string &data) {
    DUMMY_CODE(data);
    return {};
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    DUMMY_CODE(ms_since_last_tick);
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
}

void TCPConnection::connect() {
    _sender.fill_window();
    segments_out().push(_sender.segments_out().front());
    _sender.segments_out().pop();
    _linger_after_streams_finish=true;
    expectingSyn=true;
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

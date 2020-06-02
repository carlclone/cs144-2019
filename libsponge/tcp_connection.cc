#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template<typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPConnection::segment_received(const TCPSegment &seg) {
    timeSinceLastSegmentReceived = 0;

    auto header = seg.header();
    auto payload = seg.payload();

    if (state() == TCPState::State::SYN_SENT) {
        if (header.ack && header.ackno != _sender.next_seqno()) {
            return;
        }
        if ((header.rst && header.ack)) {
            doReset(false);
            return;
        }
        if (!header.syn) {
            return;
        }
    }

    if (state() == TCPState::State::LISTEN) {
        if (header.ack) {
            return;
        }
        if (!header.syn) {
            return;
        }
    }


    if (header.rst && header.seqno != _receiver.ackno()) {
        return;
    }

    if (header.rst) {
        doReset();
        return;
    }

    /*
     * if the ack flag is set, tell theTCP Sender about the fields it cares
     * about on incoming segments:ackno and window size
     */
    bool ackSuccess = false, recvSuccess = false;
    bool seqnoIsCorrect = header.seqno == _receiver.ackno();
    if (header.ack) {
        /*
         * Okay, fine.  How about if theTCPConnection received a segment, and
         * theTCPSender complains
         * that an ackno was invalid (ack received() returns false)?
         *
         * same as receiver
         */
        ackSuccess = _sender.ack_received(header.ackno, header.win);
    }

    /*
     * and give the segment to theTCPReceiver
     * so it can inspect the fields it cares about
     * on incoming segments:seqno,syn,payload, and fin
     *
     * On receiving a segment, what should I do if theTCPReceivercomplains
     * that the segmentdidn’t overlap the window and was unacceptable
     * (segmentreceived()returns false)?In that situation,
     * theTCPConnectionneeds to make sure that a segment is
     * sent back tothe peer, giving the currentacknoandwindowsize.
     * This can help correct a confusedpeer.
     */

    recvSuccess = _receiver.segment_received(seg);

    /*
     * The bottom line is that if the TCPConnection’s inbound stream ends before
        the TCPConnection has ever sent a fin segment, then the TCPConnection
        doesn’t need to linger after both streams finish.
     */
    if (_receiver.stream_out().input_ended() && !_sender.stream_in().input_ended()) {
        endBeforeFin = true;
    }

    if (ackSuccess && recvSuccess
        && seg.length_in_sequence_space() == 0
        && seqnoIsCorrect
            ) {
        return;
    }

    /*
     * TheTCPConnection will send TCPSegments over the Internet:•
     * whenever theTCPSender pushes a segment onto its outgoing queue,
     * having set the fields it’s responsible for on outgoing segments:
     * (seqno,syn,payload, andfin).
     *
     */
    _sender.fill_window();
    if (_sender.segments_out().size() == 0) {
        _sender.send_empty_segment();
    }

    sendOut();
}

void TCPConnection::askReceiver(TCPSegment &ongoingSeg) const {
    auto ackno = _receiver.ackno();
    if (ackno.has_value()) {
        ongoingSeg.header().ack = true;
        ongoingSeg.header().ackno = ackno.value();
        ongoingSeg.header().win = _receiver.window_size();
    }
}

/*
 * When this happens, the implementation releases its exclusive claim to a local
port number, stops sending acknowledgments in reply to incoming segments, considers the
connection to be history, and has its
active()
method return false
 */
bool TCPConnection::active() const {
    //unclean shutdown
    if (_sender.stream_in().error() && _receiver.stream_out().error()) {
        return false;
    }

    //clean shutdown
    if ((_sender.stream_in().eof() && _sender.bytes_in_flight() == 0) &&
        (_receiver.stream_out().input_ended()) &&
        (_linger_after_streams_finish == false || time_since_last_segment_received() >= 10 * _cfg.rt_timeout)) {
        return false;
    }

    return true;
}

size_t TCPConnection::write(const string &data) {
    auto size = _sender.stream_in().write(data);
    _sender.fill_window();
    sendOut();
    return size;
}


//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    timeSinceLastSegmentReceived += ms_since_last_tick;

    if (endBeforeFin) {
        _linger_after_streams_finish = false;
    }

    /*
     * there are two situations where you’ll want to abort the entire
     * connection:
     *
     * 1.If the sender has sent too many consecutive
     * retransmissions without success
     * (more than TCPConfig::MAXRETXATTEMPTS).
     *
     */
    _sender.tick(ms_since_last_tick);

    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        doReset();
        return;
    }


    if (state() == TCPState::State::ESTABLISHED) {
        _sender.fill_window();
    }

    if (state() == TCPState::State::TIME_WAIT) {
        if ((!_receiver.stream_out().input_ended()) &&
            (_linger_after_streams_finish == false || time_since_last_segment_received() >= 10 * _cfg.rt_timeout)) {
            //_sender.stream_in().input_ended();
            _receiver.stream_out().input_ended();
            _linger_after_streams_finish = false;
        }
    }
    sendOut();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    sendOut();
}

void TCPConnection::connect() {
    _sender.fill_window();
    sendOut();
}

TCPConnection::~TCPConnection() {

    try {
        /*
         * there are two situations where you’ll want to abort the entire
         * connection:

         * 2.If  theTCPConnectiondestructor  is  called
         * while  the  connection  is  still  active(active()returns true).
         */
        if (active()) {
            // Your code here: need to send a RST segment to the peer
            doReset();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}


// my helpers

/*
 * This flag (“reset”) means instant death to the connection.
 * If you receive a segment with rst,
 * you should set the error flag on the inbound
 * and outboundByteStreams,
 * and any subsequent call toTCPConnection::active()
 * should return false.
 */

/*
 * Wait, but how do I even make a segment that I can set
 * the rst flag on?  What’s thesequence number?
 *
 * Any outgoing segment needs to have the proper sequence number.
 * You can force theTCPSenderto generate an empty segment with
 * the proper sequence number by calling its send empty segment()
 * method.  Or you can make it fill the window
 * (generatingsegmentsifit  has  outstanding
 * information  to  send,  e.g.  bytes  from  the
 * stream  orSYN/FIN) by calling itsfillwindow()method.
 */
void TCPConnection::doReset(bool send) {
    _receiver.stream_out().set_error();
    _sender.stream_in().set_error();
    _linger_after_streams_finish = false;
    _sender.send_empty_segment();
    TCPSegment seg = _sender.segments_out().front();
    _sender.segments_out().pop();
    seg.header().rst = true;

    if (send) {
        segments_out().push(seg);
    }
}

void TCPConnection::sendOut() {
    while (!_sender.segments_out().empty()) {
        auto seg = _sender.segments_out().front();
        if (seg.header().fin) {
            cout << "sending a fin\n";
        } else {
            cout << "sending others\n";
        }
        _sender.segments_out().pop();
        /*
        * Before  sending  the  segment,  theTCPConnection
        * will  ask  theTCPReceiver for  the fields
        * it’s responsible for on outgoing segments:
        * ackno and windowsize.
        * If there is anackno,1
        * it will set theackflag and the fields in theTCPSegment.
        */
        askReceiver(seg);
        segments_out().push(seg);
    }
}

void TCPConnection::print() {
    //cout << _receiver.ackno().value();
    //cout<<_sender.consecutive_retransmissions();
    cout << "abc";
}


// official helpers

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
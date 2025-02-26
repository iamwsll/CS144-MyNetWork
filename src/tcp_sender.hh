#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>

class RetransmissionTimer
{
public:
  RetransmissionTimer(uint64_t initial_RTO_ms):_initial_RTO_ms_(initial_RTO_ms),_RTO(initial_RTO_ms){}
  void startRunning()
  {
    _timerIsRunning = true;
    _timer = 0;
  }
  void resetRTO()
  {
    _RTO = _initial_RTO_ms_;
  }
  bool isRunning()
  {
    return _timerIsRunning;
  }
  void resetTimer()
  {
    _timer = 0;
  }
  void addTimer(uint64_t ms)
  {
    _timer+=ms;
  }
  bool outOfTime()
  {
    return _timer >= _RTO;
  }
  void doubleRTO()
  {
    _RTO *=2;
  }
private:
  uint64_t _timer{};
  bool _timerIsRunning{};
  uint64_t _initial_RTO_ms_;
  uint64_t _RTO;
};


class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), _timerInside(initial_RTO_ms)
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
  Writer& writer() { return input_.writer(); }
  const Writer& writer() const { return input_.writer(); }

  // Access input stream reader, but const-only (can't read from outside)
  const Reader& reader() const { return input_.reader(); }

private:
  // Variables initialized in constructor
  ByteStream input_;
  Wrap32 isn_;
  uint64_t _lastAckSq{};
  uint64_t _lastSendSq{};
  uint64_t _consecutiveRetransmissionsNum{};
  uint64_t _windowSize{1};
  std::queue<TCPSenderMessage> _sendQueue{};
  std::queue<TCPSenderMessage> _delayQueue{};
  bool _isSYN{};
  bool _isFIN{};
  bool _zeroFlag{};

  RetransmissionTimer _timerInside;

  bool isFINmessage();
  void setFINMessage(TCPSenderMessage& message);
  void pushIntoSendQueue(TCPSenderMessage& message);
  void readAndPushMsg(TCPSenderMessage& message);
  void sendFromQueue(const TransmitFunction& transmit);
  void checkAndFixFIN(TCPSenderMessage& message);
};

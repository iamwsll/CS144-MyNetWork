#pragma once

#include "reassembler.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

class TCPReceiver
{
public:
  // Construct with given Reassembler
  explicit TCPReceiver( Reassembler&& reassembler ) : reassembler_( std::move( reassembler ) ) {}

  /*
   * The TCPReceiver receives TCPSenderMessages, inserting their payload into the Reassembler
   * at the correct stream index.
   */
  void receive(  TCPSenderMessage message );

  // The TCPReceiver sends TCPReceiverMessages to the peer's TCPSender.
  TCPReceiverMessage send() ;

  // Access the output (only Reader is accessible non-const)
   Reassembler& reassembler()  { return reassembler_; }
   const Reader& reader()const { return reassembler_.reader(); }
   Reader& reader()  { return reassembler_.reader(); }
   const Writer& writer() const{ return reassembler_.writer(); }

private:
  Reassembler reassembler_;
  bool SYN{false};
  Wrap32 ISN{ 0 };
 bool RSTflag{false};
};

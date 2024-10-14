#include "tcp_receiver.hh"
//#include<iostream>
using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if(message.SYN)
  {
    SYN = true;
    ISN = {message.seqno};
  }
  // if(message.RST)
  // cerr<<"YEs,we have RST"<<endl;
  // cerr<<"now ,I come to here"<<endl;
  if(reassembler_.reader().has_error()||RSTflag||message.RST)[[unlikely]]
  {
    RSTflag = true;
    reassembler_.reader().set_error();
    return;
  }
  //cerr<<"\n\n Rst:"<<RSTflag<<endl;
  if(SYN)[[likely]]
  //if(SYN)[[likely]]
  {
   // std::cout<<"\n\nIAMHERE"<<std::endl;
    reassembler_.insert((message.seqno+message.SYN).unwrap(ISN,reassembler_.get__first_unassembled_index())-1,message.payload,message.FIN);
  }
  // cerr<<"i have receive"<<endl;
  return;
}

TCPReceiverMessage TCPReceiver::send()
{
  // if(reassembler_.writer().is_closed())
  // {
  //   cerr<<"\n\nYES,writer have closed!"<<std::endl;
  // }
  if(reassembler_.reader().has_error())
  {
    RSTflag = true;
  }
  std::optional<Wrap32> res_ackno{Wrap32::wrap(reassembler_.get__first_unassembled_index(),ISN)+SYN+reassembler_.writer().is_closed()};
  uint16_t res_window_size { static_cast<uint16_t>(min(reassembler_.writer().available_capacity(),static_cast<uint64_t>(65535)))};
  if(!SYN)[[unlikely]]
  { 
   // cerr<<"\n\nset Rst:"<<RSTflag<<endl;
    return 
    {.window_size = res_window_size,
     .RST = RSTflag
     //.RST = reassembler_.writer().has_error()
    };
  }
 // cerr<<"\n\nset Rst:"<<RSTflag<<endl;
  return 
  {
    
    .ackno = res_ackno,
    .window_size = res_window_size,
    .RST = RSTflag
    //.RST = reassembler_.writer().has_error()
  };
}

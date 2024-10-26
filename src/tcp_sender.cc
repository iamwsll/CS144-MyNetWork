#include "tcp_sender.hh"
#include "tcp_config.hh"
#include "cmath"
#include "iostream"
using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return _lastSendSq - _lastAckSq;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return _consecutiveRetransmissionsNum;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  TCPSenderMessage message = make_empty_message();
  if(isFINmessage())setFINMessage(message);
  else readAndPushMsg(message);
  sendFromQueue(transmit);
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  return 
  {
    .seqno = Wrap32::wrap(_lastSendSq,isn_),
    .RST = input_.has_error()
  };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if(msg.RST)input_.set_error();
  if(msg.ackno)
  {
    uint64_t ackValue = msg.ackno.value().unwrap(isn_ , _lastAckSq);
    if(_lastSendSq< ackValue)return;
    if(_lastAckSq < ackValue)
    {
      if(_timerInside.isRunning()&&sequence_numbers_in_flight())_timerInside.resetTimer();
      _timerInside.resetRTO(); 
      _lastAckSq = ackValue;
      _consecutiveRetransmissionsNum = 0;
      _zeroFlag = false; 
    }
  }
  _windowSize = msg.window_size;
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  if(_timerInside.isRunning())_timerInside.addTimer(ms_since_last_tick);
  if(_timerInside.outOfTime())
  {
    while(!_delayQueue.empty())
    {
      TCPSenderMessage reSendMsg = _delayQueue.front();
      if(reSendMsg.seqno.unwrap(isn_,_lastSendSq) + reSendMsg.sequence_length()>_lastAckSq)
      {
        transmit(reSendMsg);
        _consecutiveRetransmissionsNum++;
        _timerInside.resetTimer();
        if(_windowSize!=0) _timerInside.doubleRTO();
        break;
      }
      else _delayQueue.pop();
    }
     
  }
}
//private:
bool TCPSender::isFINmessage()
{
  return !_isFIN && input_.reader().is_finished() && _windowSize > sequence_numbers_in_flight();
}

void TCPSender::setFINMessage(TCPSenderMessage& message)
{
    message.SYN = !_isSYN;
    message.FIN = _isFIN = true;
    pushIntoSendQueue(message);
    //std::cerr<<"oh , first if has been use"<<std::endl;
}
void TCPSender::pushIntoSendQueue(TCPSenderMessage& message)
{
  _lastSendSq+=message.sequence_length();
  _sendQueue.push(message);
}
void TCPSender::readAndPushMsg(TCPSenderMessage& message)
{
    uint64_t readNum = std::min({_windowSize - sequence_numbers_in_flight(),input_.reader().bytes_buffered(),_windowSize});
    if(readNum == 0)
    {
       if(!_isSYN)
      {
        message.SYN =_isSYN = true;
        pushIntoSendQueue(message);
      }
      else if(_windowSize == 0 && !_zeroFlag)
      { 
        if(_lastAckSq == _lastSendSq && input_.reader().is_finished())message.FIN = true;
        else read(input_.reader(), 1 ,message.payload); 
        _zeroFlag = true;
        pushIntoSendQueue(message);        
      }
    }

      while(readNum > 0)
      {
        uint64_t onceReadLength = std::min(readNum,TCPConfig::MAX_PAYLOAD_SIZE);
        message = make_empty_message();
        read(input_.reader(),onceReadLength,message.payload);
        message.SYN = !_isSYN;
        message.FIN = input_.reader().is_finished();
        checkAndFixFIN(message);
        if(!_isSYN)
        {
          _isSYN = true;
        }
        pushIntoSendQueue(message);
        readNum -= onceReadLength;
      }
}
void TCPSender::sendFromQueue(const TransmitFunction& transmit)
{
   while (!_sendQueue.empty())
    {
      // if(_sendQueue.front().sequence_length() + sequence_numbers_in_flight() > _windowSize)
      //   {
      //     _sendQueue.front().FIN = false;
      //     _isFIN = false;
      //   }
      if(!_timerInside.isRunning())_timerInside.startRunning();
      if(_sendQueue.front().FIN) _isFIN = true;
      transmit(_sendQueue.front());
      _delayQueue.push(_sendQueue.front());
      _sendQueue.pop();
    }
}
void TCPSender::checkAndFixFIN(TCPSenderMessage& message)
{
  if(message.sequence_length() + sequence_numbers_in_flight() > _windowSize )
  {
    message.FIN = false;
  }
}
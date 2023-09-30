#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  (void)message;
  (void)reassembler;
  (void)inbound_stream;
  // 当接收到syn消息时，设置初始序列号isn，由于syn占位，因此消息序列号+1
  if(message.SYN){
    isn = Wrap32(message.seqno);
    is_set_isn = true;
    message.seqno = message.seqno + 1;
  }
  // 当接收到fin消息时，设置结束序列号fin
  if(message.FIN){
    fin = Wrap32(message.seqno + message.payload.size());
  }
  if(is_set_isn){
    // first_index = abs_sequo - 1, abs_sequo = sequo.unwrap
    // checkpoint是已推入流中的数据长度
    reassembler.insert(message.seqno.unwrap(isn, inbound_stream.bytes_pushed()) - 1, 
                     message.payload, message.FIN, inbound_stream);
  }
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  (void)inbound_stream;
  TCPReceiverMessage tcp_receiver;
  // ack消息需要+1
  Wrap32 ack = Wrap32::wrap(inbound_stream.bytes_pushed() + 1, isn);
  // 当isn被设置时才处理ackno
  if(is_set_isn){
    // 若接到fin则需要再+1
    tcp_receiver.ackno = ack == fin ? ack + 1 : ack;
  }

  uint64_t win_size = inbound_stream.available_capacity();
  // 窗口长度限制为65535
  tcp_receiver.window_size = win_size >= UINT16_MAX ? UINT16_MAX : win_size;
  return tcp_receiver;
}

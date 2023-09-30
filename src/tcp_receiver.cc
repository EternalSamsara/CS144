#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  (void)message;
  (void)reassembler;
  (void)inbound_stream;
  if(message.SYN){
    isn = Wrap32(message.seqno);
    is_set_isn = true;
    message.seqno = message.seqno + 1;
  }
  if(message.FIN){
    fin = Wrap32(message.seqno + message.payload.size());
  }
  if(is_set_isn){
    reassembler.insert(message.seqno.unwrap(isn, inbound_stream.bytes_pushed()) - 1, 
                     message.payload, message.FIN, inbound_stream);
  }
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  (void)inbound_stream;
  TCPReceiverMessage tcp_receiver;
  Wrap32 ack = Wrap32::wrap(inbound_stream.bytes_pushed() + 1, isn);
  if(is_set_isn){
    tcp_receiver.ackno = ack == fin ? ack + 1 : ack;
  }
  uint64_t win_size = inbound_stream.available_capacity();
  tcp_receiver.window_size = win_size >= UINT16_MAX ? UINT16_MAX : win_size;
  return tcp_receiver;
}

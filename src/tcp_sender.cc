#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

TinyTimer::TinyTimer(const uint64_t init_time):
    initial_time(init_time), retransmission_time(init_time), start_state(false){}

void TinyTimer::start(){
    start_state = true;
}

void TinyTimer::stop(){
    start_state = false;
}

void TinyTimer::reset_time(){
    retransmission_time = initial_time;
}

void TinyTimer::double_RTO(const uint64_t retransmissions_num){
    retransmission_time = pow(2, retransmissions_num) * initial_time;
}

bool TinyTimer::is_timeout(const size_t ms_since_last_tick){
    retransmission_time -= ms_since_last_tick;
    return retransmission_time <= 0 ? true : false;
}

bool TinyTimer::get_state(){
    return start_state;
}

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), 
    initial_RTO_ms_( initial_RTO_ms ), 
    is_set_syn(false), 
    is_set_fin(false), 
    outstanding_bytes(0), 
    retransmissions_num(0), 
    next_seqno(0), 
    origin_win_size(1), 
    win_size(1), 
    outstanding_message(), 
    prepared_message(), 
    timer(initial_RTO_ms)
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return outstanding_bytes;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retransmissions_num;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if(prepared_message.empty()){
    return nullopt;
  }
  TCPSenderMessage sender_message(prepared_message.front());
  prepared_message.pop_front();
  if(!timer.get_state()){
    timer.start();
  }
  return sender_message;
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  (void)outbound_stream;
  while(outstanding_bytes < win_size){
    size_t len = static_cast<size_t>(win_size - outstanding_bytes);
    len = min(len, min(static_cast<size_t>(outbound_stream.bytes_buffered()), TCPConfig::MAX_PAYLOAD_SIZE));

    TCPSenderMessage message;
    // 读取数据到消息载荷
    read(outbound_stream, len, message.payload);

    // 首次处理数据流时设置syn（syn需要占用空间）
    if(!is_set_syn){
      is_set_syn = true;
      message.SYN = true;
      message.seqno = isn_;
    }
    else{
      message.seqno = Wrap32::wrap(next_seqno, isn_);
    }

    // 处理到数据流末尾且窗口长度能容纳fin（fin需要占用空间）时设置fin
    if(!is_set_fin && outbound_stream.is_finished() && 
       message.sequence_length() + outstanding_bytes < win_size){
      is_set_fin = true;
      message.FIN = true;
    }

    // 当没有数据被放入TCPSenderMessage弹出循环
    // 因为要假设win_size=1，可能进入无限循环
    if(message.sequence_length() == 0){
      break;
    }

    // 消息推入待发送队列
    prepared_message.push_back(message);
    // 消息推入未处理队列（消息副本，用于重发处理）
    outstanding_message.push_back(message);
    // 增加序列号长度
    next_seqno += message.sequence_length();
    // 增加未处理字节的长度
    outstanding_bytes += message.sequence_length();
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  TCPSenderMessage sender_message;
  sender_message.seqno = Wrap32::wrap(next_seqno, isn_);
  return sender_message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  (void)msg;
  // 设置窗口至少长度为1
  // 目的是为了当接收方没有空间而发送方数据未被全部确认时，发送方能继续push其他数据
  win_size = msg.window_size == 0 ? 1 : msg.window_size;
  // 保存原始窗口大小，用于判断是否进行指数backoff
  origin_win_size = msg.window_size;

  // 接收的确认消息没有序列号时退出
  if(!msg.ackno.has_value()){
    return;
  }

  // 接收的确认消息的序列号需要转化为绝对序列号
  uint64_t ack_seqno = msg.ackno.value().unwrap(isn_, next_seqno);
  // 接收的确认消息的序列号若超出当前需要的序列号则不处理
  if(ack_seqno > next_seqno){
    return;
  }

  // 遍历未处理数据队列
  while(outstanding_bytes > 0){
    TCPSenderMessage outstanding = outstanding_message.front();
    uint64_t msg_abs_seqno = outstanding.seqno.unwrap(isn_, next_seqno);
    msg_abs_seqno += outstanding.sequence_length();
    // 将序列号小于接收的确认消息的序列号的数据弹出
    if(msg_abs_seqno <= ack_seqno){
      outstanding_bytes -= outstanding.sequence_length();
      outstanding_message.pop_front();
      // 若不存在未处理数据则关闭计时器，否则重启计时器
      if(outstanding_bytes == 0){
        timer.stop();
      }
      else{
        timer.start();
      }
      timer.reset_time();
      retransmissions_num = 0;
    }
    else{
      break;
    }
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  (void)ms_since_last_tick;
  // 当超时时重新将消息放入待发送消息队列中重新转发
  if(timer.get_state() && timer.is_timeout(ms_since_last_tick)){
    // 将未能及时确认的消息放入待发送队列队头中进行重新发送
    prepared_message.push_front(outstanding_message.front());
    // 重发次数+1，计算指数backoff
    retransmissions_num += 1;

    // 当接收方窗口还有空间时执行指数backoff
    // 这是假设对方网络不佳导致ack消息未能及时到达时，发送方所作出的等待策略
    // 若接收方窗口没有空间，则重置计时器为初始值，此时不应进行指数backoff
    if(origin_win_size > 0){
      timer.double_RTO(retransmissions_num);
    }
    else{
      timer.reset_time();
    }
  }
}

#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include<deque>

class TinyTimer{
public:
    TinyTimer(const uint64_t init_time);
    void start();
    void stop();
    void reset_time();
    void double_RTO(const uint64_t retransmissions_num);
    bool is_timeout(const size_t ms_since_last_tick);
    bool get_state();
private:
    uint64_t initial_time;
    int64_t retransmission_time;
    bool start_state;
};

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  bool is_set_syn;
  bool is_set_fin;
  uint64_t outstanding_bytes;
  uint64_t retransmissions_num;
  uint64_t next_seqno;
  uint64_t origin_win_size;
  uint16_t win_size;
  std::deque<TCPSenderMessage> outstanding_message;
  std::deque<TCPSenderMessage> prepared_message;
  TinyTimer timer;

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};

#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  (void)n;
  (void)zero_point;
  return Wrap32(zero_point + static_cast<uint32_t>(n));
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  (void)zero_point;
  (void)checkpoint;
  uint64_t abs_seqno = static_cast<uint64_t>(this->raw_value_ - zero_point.raw_value_);
  uint64_t mod = (checkpoint - abs_seqno) >> 32;
  uint64_t remainder = ((checkpoint - abs_seqno) << 32) >> 32;

  if(checkpoint < abs_seqno){
    return abs_seqno;
  }
  else if(remainder < (1UL << 31)){
    return abs_seqno + (mod << 32);
  }
  else{
    return abs_seqno + ((mod + 1) << 32);
  }
}

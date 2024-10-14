#include "wrapping_integers.hh"
#include <limits>
using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return zero_point + static_cast<uint32_t>(n);
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // if(raw_value_>=zero_point.raw_value_)return checkpoint + raw_value_-zero_point.raw_value_;
  // else return checkpoint + zero_point.raw_value_ - raw_value_ + (static_cast<uint64_t>(1)<<32);
  uint64_t sub{static_cast<uint64_t>(raw_value_ - wrap(checkpoint,zero_point).raw_value_)};
  uint64_t res{sub + checkpoint};
  if((sub >=static_cast<uint64_t>(1)<<31) &&
     res>=(static_cast<uint64_t>(1)<<32))[[unlikely]]
  res -= static_cast<uint64_t>(1)<<32;

  return res;
}

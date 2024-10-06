#include "byte_stream.hh"
#include<iostream>
using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : error_( false )
  , capacity_( capacity )
  , bytes_had_pushed_( 0 )
  , bytes_had_poped_( 0 )
  , is_closed_( false )
  , buffer_()
{}

bool Writer::is_closed() const
{
  // Your code here.
  return is_closed_;
}

void Writer::push( string data )
{
  // Your code here.
  uint64_t bytes_to_push = std::min( available_capacity(), static_cast<uint64_t>( data.size() ) );
  buffer_.insert( buffer_.end(), data.begin(), data.begin() + bytes_to_push );
  bytes_had_pushed_ += bytes_to_push;
  return;
}

void Writer::close()
{
  // Your code here.
  is_closed_ = true;
  return;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_had_pushed_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return is_closed_ && buffer_.empty();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_had_poped_;
}

string_view Reader::peek() const
{
  // Your code here.
  return std::string_view( &buffer_[0], 1 );
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  uint64_t bytes_to_pop = std::min( len, static_cast<uint64_t>( buffer_.size() ) );
/*std::cerr<<" i will read:";
for(auto it = buffer_.begin();it!=buffer_.begin() + bytes_to_pop;it++)
{
  std::cerr<<*it;
}
std::cerr<<std::endl;*/
  buffer_.erase( buffer_.begin(), buffer_.begin() + bytes_to_pop );

  bytes_had_poped_ += bytes_to_pop;
  //bytes_had_pushed_ -= bytes_to_pop;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer_.size();
}

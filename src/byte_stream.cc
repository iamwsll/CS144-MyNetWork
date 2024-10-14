// #include "byte_stream.hh"
// #include<iostream>
// using namespace std;

// ByteStream::ByteStream( uint64_t capacity )
//   : error_( false )
//   , capacity_( capacity )
//   , bytes_had_pushed_( 0 )
//   , bytes_had_poped_( 0 )
//   , is_closed_( false )
//   , buffer_()
// {}

// bool Writer::is_closed() const
// {
//   // Your code here.
//   //cerr<<"\n\n I am is_close and"<<is_closed_ <<endl;
//   return is_closed_;
// }

// void Writer::push( const string& data )
// {
//   // Your code here.
//   uint64_t bytes_to_push = std::min( available_capacity(), static_cast<uint64_t>( data.size() ) );
//   buffer_.insert( buffer_.end(), data.begin(), data.begin() + bytes_to_push );
//   bytes_had_pushed_ += bytes_to_push;
//   return;
// }

// void Writer::close()
// {
//   // Your code here.
//   cerr<<"\n\n I am close"<<endl;
//   is_closed_ = true;
//   return;
// }

// uint64_t Writer::available_capacity() const
// {
//   // Your code here.
//   return capacity_ - buffer_.size();
// }

// uint64_t Writer::bytes_pushed() const
// {
//   // Your code here.
//   return bytes_had_pushed_;
// }

// bool Reader::is_finished() const
// {
//   // Your code here.
//   return is_closed_ && buffer_.empty();
// }

// uint64_t Reader::bytes_popped() const
// {
//   // Your code here.
//   return bytes_had_poped_;
// }

// string_view Reader::peek() const
// {
//   // Your code here.
//   return std::string_view( &buffer_[0], 1 );
// }

// void Reader::pop( uint64_t len )
// {
//   // Your code here.
//   uint64_t bytes_to_pop = std::min( len, static_cast<uint64_t>( buffer_.size() ) );
// /*std::cerr<<" i will read:";
// for(auto it = buffer_.begin();it!=buffer_.begin() + bytes_to_pop;it++)
// {
//   std::cerr<<*it;
// }
// std::cerr<<std::endl;*/
//   buffer_.erase( buffer_.begin(), buffer_.begin() + bytes_to_pop );

//   bytes_had_poped_ += bytes_to_pop;
//   //bytes_had_pushed_ -= bytes_to_pop;
// }

// uint64_t Reader::bytes_buffered() const
// {
//   // Your code here.
//   return buffer_.size();
// }
#include "byte_stream.hh"
#include <string>
#include <string_view>

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
    : error_( false )
    , capacity_( capacity )
    , bytes_had_pushed_( 0 )
    , bytes_had_poped_( 0 )
    , is_closed_( false )
    , buffer_()
    , read_pos_( 0 )
{}

Reader& ByteStream::reader() { return static_cast<Reader&>( *this ); }
const Reader& ByteStream::reader() const { return static_cast<const Reader&>( *this ); }
Writer& ByteStream::writer() { return static_cast<Writer&>( *this ); }
const Writer& ByteStream::writer() const { return static_cast<const Writer&>( *this ); }

void Writer::push( const std::string& data )
{
    uint64_t bytes_to_push = min( available_capacity(), static_cast<uint64_t>( data.size() ) );
    buffer_.append( data.substr( 0, bytes_to_push ) );
    bytes_had_pushed_ += bytes_to_push;
}

void Writer::close()
{
    is_closed_ = true;
}

bool Writer::is_closed() const
{
    return is_closed_;
}

uint64_t Writer::available_capacity() const
{
    return capacity_ - ( buffer_.size() - read_pos_ );
}

uint64_t Writer::bytes_pushed() const
{
    return bytes_had_pushed_;
}

std::string_view Reader::peek() const
{
    return std::string_view( buffer_.data() + read_pos_, buffer_.size() - read_pos_ );
}

void Reader::pop( uint64_t len )
{
    uint64_t bytes_to_pop = min( len, buffer_.size() - read_pos_ );
    read_pos_ += bytes_to_pop;
    bytes_had_poped_ += bytes_to_pop;

    if ( read_pos_ == buffer_.size() ) [[unlikely]]
    {
        buffer_.clear();
        read_pos_ = 0;
    }
}

bool Reader::is_finished() const
{
    return is_closed_ && ( buffer_.size() - read_pos_ == 0 );
}

uint64_t Reader::bytes_buffered() const
{
    return buffer_.size() - read_pos_;
}

uint64_t Reader::bytes_popped() const
{
    return bytes_had_poped_;
}

void read( Reader& reader, uint64_t len, std::string& out )
{
    std::string_view data = reader.peek();
    uint64_t bytes_to_read = min( len, static_cast<uint64_t>( data.size() ) );
    out.append( data.substr( 0, bytes_to_read ) );
    reader.pop( bytes_to_read );
}

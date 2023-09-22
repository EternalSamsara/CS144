#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  // Your code here.
  if(data.empty() || available_capacity() == 0){
    return;
  }
  size_t n = min(available_capacity(), data.length());
  if(n < data.length()){
    data = data.substr(0, n);
  }
  data_buffer.emplace(data);
  pushed_length += n;
  buffered_length += n;
}

void Writer::close()
{
  // Your code here.
  is_close = true;
}

void Writer::set_error()
{
  // Your code here.
  is_error= true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return is_close;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - buffered_length;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return pushed_length;
}

string_view Reader::peek() const
{
  // Your code here.
  if(data_buffer.empty()){
    return {};
  }
  return string_view(data_buffer.front());
}

bool Reader::is_finished() const
{
  // Your code here.
  return buffered_length == 0 && is_close;
}

bool Reader::has_error() const
{
  // Your code here.
  return is_error;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  (void)len;
  size_t n = min(len, buffered_length);
  while(n > 0){
    size_t str_length = data_buffer.front().length();
    if(n < str_length){
      // 需要弹出的数据长度小于队列头部的数据长度时,对队列头部的数据只裁减不弹出
      data_buffer.front().erase(0, n);
      buffered_length -= n;
      poped_length += n;
      break;
    }
    data_buffer.pop();
    n -= str_length;
    buffered_length -= str_length;
    poped_length += str_length;
  }
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffered_length;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return poped_length;
}

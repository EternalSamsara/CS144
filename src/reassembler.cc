#include "reassembler.hh"

using namespace std;

Reassembler::Reassembler():data_deque(), flag_deque(), init_info(true), 
  unassemble_idx(-1), unacceptable_idx(-1), package_end(-1){}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  (void)first_index;
  (void)data;
  (void)is_last_substring;
  (void)output;

  if(init_info){
    data_deque.resize(output.available_capacity(), '\0');
    flag_deque.resize(output.available_capacity(), false);
    init_info = false;
  }

  if(is_last_substring){
    package_end = first_index + data.size();
  }

  // 未排序子串索引起始位置
  unassemble_idx = output.bytes_pushed();
  // 未排序子串索引结束位置
  unacceptable_idx = unassemble_idx + output.available_capacity();

  uint64_t cur_data_end = first_index + data.size(), sub_start = 0, sub_end = 0;

  // 丢弃尾部索引在已排序范围内和头部索引在流可接收范围外的字串
  if(data.empty() || cur_data_end < unassemble_idx || first_index >= unacceptable_idx){
    data = "";
  }
  else{
    sub_start = first_index;
    sub_end = cur_data_end;

    // 对传入子串在未排序范围内掐头去尾
    if(sub_start < unassemble_idx){
      sub_start = unassemble_idx;
    }
    if(sub_end > unacceptable_idx){
      sub_end = unacceptable_idx;
    }
    
    for(size_t i = sub_start; i < sub_end; i++){
      data_deque[i - unassemble_idx] = data[i - first_index];
      flag_deque[i - unassemble_idx] = true;
    }
  }

  string tmp = "";
  while(flag_deque.front()){
    // 从队头开始只拼接该位置为true的字符到tmp
    tmp += data_deque.front();

    data_deque.pop_front();
    flag_deque.pop_front();
    
    data_deque.push_back('\0');
    flag_deque.push_back(false);
  }

  // 写入byteStream
  if(!tmp.empty()){
    output.push(tmp);
  }

  // 当cur_data_idx到达最后一个子串的末尾时关闭流
  if(output.bytes_pushed() == package_end){
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return static_cast<uint64_t>(count(flag_deque.begin(), flag_deque.end(), true));
}

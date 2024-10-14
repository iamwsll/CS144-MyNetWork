#pragma once

#include "byte_stream.hh"
#include<set>
class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  explicit Reassembler(ByteStream&& output)
    : output_(std::move(output)),
      _subStringSet(),
      _first_unpopped_index(0),
      _first_unassembled_index(0),
      _first_unacceptable_index(0),
      _bytes_pending(0),
      _is_last_input(false) {}

  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, const std::string& data, bool is_last_substring );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }
  uint64_t get__first_unassembled_index(){return _first_unassembled_index;}
private:
   class SubStringInsert
{
  public:
    SubStringInsert(uint64_t index,const std::string& data)
    :_index(index),_data(data),_length(data.size()){}
    const uint64_t& getIndex()const{return _index;}
    const std::string& getString()const{return _data;}
    const uint64_t& getLength()const{return _length;}
    void setIndex(uint64_t index){_index = index;}
    void setData(std::string data){_data = data;}
    void updateLength(){_length = _data.size();}
    bool operator< (const SubStringInsert& s)const{return _index<s._index;}
  private:
    uint64_t _index;
    std::string _data;
    uint64_t _length;
};
  ByteStream output_; // the Reassembler writes to this ByteStream
  std::set<SubStringInsert> _subStringSet;
  uint64_t _first_unpopped_index;
  uint64_t _first_unassembled_index;
  uint64_t _first_unacceptable_index;
  uint64_t _bytes_pending;
  bool _is_last_input;

  void cut_off_substring(SubStringInsert& new_substring,bool is_last_substring);
  void update_assembly(SubStringInsert& new_substring);
  void push_assembly();
};

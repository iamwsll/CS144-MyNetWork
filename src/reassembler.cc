#include "reassembler.hh"
//#include<iostream>
using namespace std;

void Reassembler::insert( uint64_t first_index, const string& data, bool is_last_substring )
{
    SubStringInsert current_substring(first_index,data);
    _first_unpopped_index = output_.reader().bytes_popped();
    _first_unacceptable_index = _first_unpopped_index + output_.writer().available_capacity() + output_.reader().bytes_buffered();
    //std::cerr<<"now insert is "<<data<<" and unpoped_index unacceptable_index is"<<_first_unpopped_index<<' '<<_first_unacceptable_index<<std::endl;
    cut_off_substring(current_substring,is_last_substring);
    //std::cerr<<"now the substring become " <<current_substring.getString()<<std::endl;
    update_assembly(current_substring);
    push_assembly();
    if(_subStringSet.empty()&&_is_last_input)[[unlikely]]
    {

      output_.writer().close();
    }
}
void Reassembler::cut_off_substring(SubStringInsert& new_substring,bool is_last_substring)
{
  if(new_substring.getIndex()>=_first_unacceptable_index)[[unlikely]]
  {
    new_substring.setData("");
    new_substring.updateLength();
    _is_last_input = _is_last_input||is_last_substring;
    return;
  }
  int64_t bytes_overflow = new_substring.getIndex()+new_substring.getLength()-_first_unacceptable_index;
  if(bytes_overflow>0)[[unlikely]]
  {
    int64_t new_length = new_substring.getLength() - bytes_overflow;
    if(new_length<=0)[[unlikely]]
    {
      new_substring.setData("");
      new_substring.updateLength();
      is_last_substring = false;
    }
    else[[likely]]
    {
      new_substring.setData(new_substring.getString().substr(0,new_length));
      new_substring.updateLength();
      is_last_substring = false;
    }
  } 
  if(new_substring.getIndex()<_first_unassembled_index)[[unlikely]]
  {
    int64_t new_length = new_substring.getIndex()+new_substring.getLength() - _first_unassembled_index;
    if(new_length<=0)[[unlikely]]
    {
      new_substring.setData("");
      new_substring.updateLength();
      new_substring.setIndex(_first_unassembled_index);
    }
    else[[likely]]
    {
      new_substring.setData(new_substring.getString().substr(_first_unassembled_index-new_substring.getIndex(),new_length));
      new_substring.updateLength();
      new_substring.setIndex(_first_unassembled_index);
    }
  }
  _is_last_input = _is_last_input||is_last_substring;
}
void Reassembler::update_assembly(SubStringInsert& new_substring)
{
  int64_t bytes_changed = 0;
  //std::cerr<<"i will update :"<<new_substring.getString()<<std::endl;
  for(auto it = _subStringSet.begin();it!=_subStringSet.end();)
  {

    if(new_substring.getIndex()>=it->getIndex()&&new_substring.getIndex()<it->getIndex()+it->getLength())[[unlikely]]
    {
      if(new_substring.getIndex()+new_substring.getLength()<=it->getIndex()+it->getLength())
      {
        new_substring.setIndex(it->getIndex());
        new_substring.setData(it->getString());
        new_substring.updateLength();
      }
      else
      {
        new_substring.setData(it->getString().substr(0,new_substring.getIndex()-it->getIndex())+new_substring.getString());
        new_substring.setIndex(it->getIndex());
        new_substring.updateLength();
       // std::cerr<<"it is "<<it->getString()<<" and "<<new_substring.getIndex() <<' '<<it->getIndex()<<std::endl;
       // std::cerr<<"substring is"<<new_substring.getString()<<std::endl;
        //std::cerr<<"it->getString().substr ->"<<it->getString().substr(0,new_substring.getIndex()-it->getIndex())<<std::endl;
      }
     // bytes_changed+=new_substring.getLength();
      bytes_changed-=it->getLength();
       it = _subStringSet.erase(it);
     // std::cerr<<"now,substring become: "<<new_substring.getString()<<std::endl;
    }
    else if(new_substring.getIndex()<=it->getIndex()&&it->getIndex()<new_substring.getIndex()+new_substring.getLength())[[unlikely]]
    {
      if(new_substring.getIndex()+new_substring.getLength()<=it->getIndex()+it->getLength())
      {
        uint64_t begin_sub_index = new_substring.getIndex()+new_substring.getLength()-it->getIndex();
        uint64_t end_index = new_substring.getIndex()+new_substring.getLength()-(it->getIndex()+it->getLength());
        new_substring.setData(new_substring.getString()+it->getString().substr(begin_sub_index,end_index));
        new_substring.updateLength();
      }
     // bytes_changed+=new_substring.getLength();
      bytes_changed-=it->getLength();
       it = _subStringSet.erase(it);
      //std::cerr<<"now,substring become: "<<new_substring.getString()<<std::endl;
    }
    else
    {
      ++it;
    }
  }
  bytes_changed+=new_substring.getLength();
  _bytes_pending=static_cast<int64_t>(_bytes_pending)+bytes_changed;
  _subStringSet.insert(new_substring);
  //std::cerr<<"now,the _bytes_pending ->"<<_bytes_pending<<std::endl;
}
void Reassembler::push_assembly()
{
  while(!_subStringSet.empty()&&_subStringSet.begin()->getIndex()==_first_unassembled_index)
  {
    output_.writer().push(_subStringSet.begin()->getString());
    //std::cerr<<"i have write "<<_subStringSet.begin()->getString()<<" now ,bytes_push is"<<output_.writer().bytes_pushed()<<std::endl;
    _first_unassembled_index+=_subStringSet.begin()->getLength();
   // std::cerr<<"now,the _bytes_pending ->"<<_bytes_pending<<std::endl;
    _bytes_pending-=_subStringSet.begin()->getLength();
    _subStringSet.erase(_subStringSet.begin());
  }
}
uint64_t Reassembler::bytes_pending() const
{
  return _bytes_pending;
}

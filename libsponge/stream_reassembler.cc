#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}
using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : 
    _output(capacity), 
    _capacity(capacity), 
    _buffer(),
    _eof(false),
    _eof_idx(0),
    _unassembled_bytes(0)
{

}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.


void StreamReassembler::handle_substring(const string &data, const size_t index){
    
    auto seg = Segment{index, data};

    // 范围是[read_size_ , read_size_ + _capacity - 1],第一个不可以被写入的索引是 read_size_ + _capacity
    //      [index, index + data.length() - 1]

    // seg落在可写区域的外面 ok
    if(seg.idx_ >= _output.bytes_read() + _capacity){
        return;
    }

    // seg.data_右端如果超出了右边界
    if(seg.idx_ < _output.bytes_read() + _capacity &&
       seg.idx_ + seg.data_.length() - 1 >= _output.bytes_read() + _capacity){
        // [seg.idx_, read_size_ + _capacity - 1]
        seg.data_ = seg.data_.substr(0, _output.bytes_read() + _capacity - seg.idx_);
    }

    // 如果seg.data_都已经写入了ByteSream中 ok
    if(seg.idx_ + seg.data_.length() - 1 < _output.bytes_written()){
        return;
    }

    // 如果seg.data_有一部分已被写入，另一部分没有被写入
    if(seg.idx_ < _output.bytes_written() && 
       seg.idx_ + seg.data_.length() - 1 >= _output.bytes_written()){
        seg.data_ = seg.data_.substr(_output.bytes_written() - seg.idx_);
        seg.idx_ = _output.bytes_written();
    }

    if(_buffer.empty()){
        buffer_insert(seg);
        return;
    }

    handle_overlap(seg);
}



void StreamReassembler::adjustment(Segment& seg, const std::set<Segment>::iterator& it){
    // [it_l , it_r]
    size_t it_l = it->idx_;
    size_t it_r = it->idx_ + it->data_.length() - 1;
    // [seg_l, seg_r]
    size_t seg_l = seg.idx_;
    size_t seg_r = seg.idx_ + seg.data_.length() - 1;

    // 1. 如果 seg 全包含 it 的范围     
    // _buffer去除 it
    if(seg_l <= it_l && seg_r >= it_r){
        return;
    }

    // 2. 如果it全包含 seg 的范围
    // seg 变为 it, _buffer删除 it 
    if(it_l <= seg_l && it_r >= seg_r){
        seg.idx_ = it_l;
        seg.data_ = it->data_;
        return;
    }

    // 3. 如图下：
    /*
        seg:           _________
        it:        _______
    */
   
    if(seg_l >= it_l && seg_r > it_r){
        seg.data_ = it->data_ + seg.data_.substr(it->idx_ + it->data_.length() - seg.idx_);
        seg.idx_ = it->idx_;
        return;
    }
    


    // 4. 如下图：
     /*
        seg:      _________
        it:            ________
    */
    
    if(it_l > seg_l && it_r >= seg_r){
        seg.data_ = seg.data_.substr(0, it->idx_ - seg.idx_) + it->data_;
        return;
    }

}


void StreamReassembler::handle_overlap(Segment& seg){
    // 保证插入的和原本存在的没有重叠部分
    auto it = _buffer.begin();
    for(; it != _buffer.end();){
        size_t it_l = it->idx_;
        size_t it_r = it->idx_ + it->data_.length() - 1;

        size_t seg_l = seg.idx_;
        size_t seg_r = seg.idx_ + seg.data_.length() - 1;

        // 两条线段有重叠
        if((it_l >= seg_l && it_l <= seg_r) || (seg_l >= it_l && seg_l <= it_r)){
            adjustment(seg, it);
            buffer_erase(it++);
        }else{
            it++;
        }
    }

    buffer_insert(seg);
}

void StreamReassembler::buffer_erase(const std::set<Segment>::iterator& it){
    // 如果删除了，那么 _unassembled_bytes 就要减
    _unassembled_bytes -= it->data_.length();
    _buffer.erase(it);
}

void StreamReassembler::buffer_insert(Segment& seg){
    _unassembled_bytes += seg.data_.length();
    _buffer.insert(seg);
}


void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {

    if(!data.empty()){
        handle_substring(data, index);
    }

    // 检查 _buffer 中是否有连续的可写入 ByteStream 的部分
    // 第一个可以写入的字节是 written_size_
    while(!_buffer.empty() && _buffer.begin()->idx_ == _output.bytes_written()){
        auto it = _buffer.begin();
        _output.write(it->data_);
        buffer_erase(it);
    }

    if(eof){
        _eof = true;
        _eof_idx = index + data.length();
    }

    if(_eof && _output.bytes_written() == _eof_idx){
        _output.end_input();
    }

}


size_t StreamReassembler::unassembled_bytes() const { 
    return _unassembled_bytes;
}


bool StreamReassembler::empty() const { 
    return _buffer.empty();
}

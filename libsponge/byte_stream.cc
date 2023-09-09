#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity):dq_(), capacity_(capacity), read_size_(0), write_size_(0),end_input_(0) { 
    
}

size_t ByteStream::write(const string &data) {
    if(end_input_){
        return 0;
    }

    // 把 data 写进 dq_中
    size_t write_sz = std::min(data.size(), capacity_ - dq_.size());
    write_size_ += write_sz;

    for(size_t i = 0; i < write_sz; i++){
        dq_.push_back(data[i]);
    }

    return write_sz;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    // dq_中的内容不会被清除
    size_t pop_size = min(len, dq_.size());
    return string(dq_.begin(), dq_.begin() + pop_size);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    //dq_中的内容会被清除
    size_t pop_size = min(len, dq_.size());
    read_size_ += pop_size;
    for(size_t i = 0; i < pop_size; i++){
        dq_.pop_front();
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    std::string ret = peek_output(len);
    pop_output(len);
    return ret;
}

void ByteStream::end_input() {
    end_input_ = true;
}

bool ByteStream::input_ended() const { 
    return end_input_;    
}

size_t ByteStream::buffer_size() const { 
    return dq_.size();
}

bool ByteStream::buffer_empty() const { 
    return dq_.size() == 0;
}

bool ByteStream::eof() const { 
    return dq_.size() == 0 && input_ended();    
}

size_t ByteStream::bytes_written() const { 
    return write_size_;
}

size_t ByteStream::bytes_read() const { 
    return read_size_;
}

size_t ByteStream::remaining_capacity() const { 
    return capacity_ - dq_.size();    
}

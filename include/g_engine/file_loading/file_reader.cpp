#include "file_reader.hpp"
#include <cstdint>
#include <fstream>
#include <sstream>
#define SwapTwoBytes(data) \
( (((data) >> 8) & 0x00FF) | (((data) << 8) & 0xFF00) ) 

#define SwapFourBytes(data)   \
( (((data) >> 24) & 0x000000FF) | (((data) >>  8) & 0x0000FF00) | \
  (((data) <<  8) & 0x00FF0000) | (((data) << 24) & 0xFF000000) ) 

#define SwapEightBytes(data)   \
( (((data) >> 56) & 0x00000000000000FF) | (((data) >> 40) & 0x000000000000FF00) | \
  (((data) >> 24) & 0x0000000000FF0000) | (((data) >>  8) & 0x00000000FF000000) | \
  (((data) <<  8) & 0x000000FF00000000) | (((data) << 24) & 0x0000FF0000000000) | \
  (((data) << 40) & 0x00FF000000000000) | (((data) << 56) & 0xFF00000000000000) ) 


gore::filereader::filereader (std::string file_name) {
    this->file_name = file_name;
    std::ifstream file (file_name, std::ios::binary);
    offset = 0;
    size = 0;
    if (!file) {
        return;
    }
    std::stringstream stream;
    stream << file.rdbuf();
    std::string str = stream.str();
    this->data = str;
    this->size = data.size();
}
// copy
gore::filereader::filereader(const filereader& f) {
    this->file_name = f.file_name;
    this->offset = f.offset;
    this->size = f.size;
    this->data = f.data;
}
// move
gore::filereader::filereader(const filereader&& f) {
    this->file_name = f.file_name;
    this->offset = f.offset;
    this->size = f.size;
    std::move(f.data.begin(), f.data.end(), this->data.begin());
}

gore::filereader::filereader(void* data, size_t size) {
    this->data = std::string((char*)data, size);
    this->offset = 0;
    this->size = size;
}

void gore::filereader::moveHeadBack (uint32_t n) {
    offset -= n;
}
void gore::filereader::moveHeadForward (uint32_t n) {
    offset += n;
}
void gore::filereader::resetHead () {
    offset = 0;
}
void gore::filereader::setHead (uint32_t n) {
    offset = n;
}
uint32_t gore::filereader::fileSize () {
    return size;
}
char* gore::filereader::getHead() {
    return (this->data.data()) + this->offset;
}
uint32_t gore::filereader::getOffset () {
    return this->offset;
}

uint32_t gore::filereader::readFourBytes (bool big_endian) {
    if (this->offset >= this->size) {
        return 0;
    }
    uint32_t val = *(uint32_t*)(this->data.data() + this->offset);
    if (big_endian) {
        val = SwapFourBytes(val);
    }
    this->offset += sizeof(uint32_t);
    return val;
}
uint16_t gore::filereader::readTwoBytes (bool big_endian) {
    if (this->offset >= this->size) {
        return 0;
    }
    uint16_t val = *(uint16_t*)(this->data.data() + this->offset);
    if (big_endian) {
        val = SwapTwoBytes(val);
    }
    this->offset += sizeof(uint16_t);
    return val;
}
uint8_t gore::filereader::readOneByte () {
    if (this->offset >= this->size) {
        return 0;
    }
    uint8_t val = *(uint8_t*)(this->data.data() + this->offset);
    this->offset += sizeof(uint8_t);
    return val;
}
#pragma once
#include <cstdint>
#include <string>

namespace gore {
// read files byte by byte
class filereader {
private:
    uint32_t offset;
    uint32_t size;
    std::string data;
public:
    std::string file_name;    
    filereader (std::string file_name);
    // copy
    filereader(const filereader& f);
    // move
    filereader(const filereader&& f);
    // wrap raw data
    filereader(void* data, size_t size);

    void moveHeadBack (uint32_t n);
    void moveHeadForward (uint32_t n);
    void resetHead ();
    void setHead (uint32_t n);
    uint32_t fileSize ();
    char* getHead();
    uint32_t getOffset();

    uint32_t readFourBytes (bool big_endian);
    uint16_t readTwoBytes (bool big_endian);
    uint8_t readOneByte ();
};
}
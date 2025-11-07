#pragma once
#include <cstdint>
#include <string>


// read files byte by byte
class FileReader {
private:
    uint32_t offset;
    uint32_t size;
    std::string data;
public:
    std::string file_name;    
    FileReader (std::string file_name);
    // copy
    FileReader(const FileReader& f);
    // move
    FileReader(const FileReader&& f);

    void moveHeadBack (uint32_t n);
    void moveHeadForward (uint32_t n);
    void resetHead ();
    void setHead (uint32_t n);
    uint32_t fileSize ();
    char* getHead();

    uint32_t readFourBytes (bool big_endian);
    uint16_t readTwoBytes (bool big_endian);
    uint8_t readOneByte ();
};
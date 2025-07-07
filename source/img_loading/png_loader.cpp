#include "image_loader.hpp"
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <new>

#define FLIP_ENDIAN_32(x) ( \
    (((x) >> 24) & 0x000000FF) | \
    (((x) >> 8)  & 0x0000FF00) | \
    (((x) << 8)  & 0x00FF0000) | \
    (((x) << 24) & 0xFF000000) )

PREVENT_PACKING_STRUCT IHDR {
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t compression;
    uint8_t filter;
    uint8_t interlace;
};
END_PACKING_STRUCT

void readInto (char* target, char* buffer, uintmax_t buffer_size, uintmax_t start, uintmax_t size) {
    for (uintmax_t i = 0, j = start; i < size && j < buffer_size; i++, j++) {
        target[i] = buffer[j];
    }
}


IMG imageloader::loadPNG(std::string path, unsigned int w, unsigned int h) {
    // open file
    std::ifstream file;
    file.open(path, std::ios::binary);
    uintmax_t file_size = std::filesystem::file_size(path);
    char* buffer = new char [file_size];
    file.read(buffer, file_size);
    file.close();
    if (file_size < 67) {
        // below minimum size
        delete[] buffer;
        return nullptr;
    }
    // read the header
    uint32_t* buffer_ptr = (uint32_t*)buffer;
    uint32_t val = *buffer_ptr;
    if (val != 0x474E5089) {
        delete[] buffer;
        return nullptr; // missing first four bytes
    }
    buffer_ptr++;
    val = *buffer_ptr;
    if (val != 0x0A1A0A0D) {
        delete[] buffer;
        return nullptr; // missing second four bytes
    }
    buffer_ptr++;
    val = *buffer_ptr;
    uint32_t length = FLIP_ENDIAN_32(val);
    if (length != 13) {
        return nullptr; // IHDR is wrong
    }
    buffer_ptr++;
    val = *buffer_ptr;
    if (val != 0x52444849) {
        return nullptr; // IHDR tag is wrong
    }
    buffer_ptr++;
    IHDR ihdr;
    readInto((char*)&ihdr, buffer, file_size, 16, 13);
    ihdr.width = FLIP_ENDIAN_32(ihdr.width);
    ihdr.height = FLIP_ENDIAN_32(ihdr.height);
    switch (ihdr.color_type) {
        case 0:
        
        break;
        case 2:
        break;
        case 3:
        break;
        case 4:
        break;
        case 6:
        break;
    }
    return nullptr;
}
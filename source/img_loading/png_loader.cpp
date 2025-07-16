#include "image_loader.hpp"
#include "inflate.hpp"
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <new>


#define PNG_SIGNATURE_FIRST_FOUR 0x474E5089
#define PNG_SIGNATURE_SECOND_FOUR 0x0A1A0A0D
#define PNG_IHDR_TAG 0x52444849
#define PNG_PLTE_TAG 0x454C5450 // this is wrong
#define PNG_IDAT_TAG 0x54414449

#define FLIP_ENDIAN_32(x) ( \
    (((x) >> 24) & 0x000000FF) | \
    (((x) >> 8)  & 0x0000FF00) | \
    (((x) << 8)  & 0x00FF0000) | \
    (((x) << 24) & 0xFF000000) )

#define READ_AS_UINT32(x) *((uint32_t*)(x))

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

enum FilterType {FILTER_NONE = 0, FILTER_SUB = 1, FILTER_UP = 2, FILTER_AVERAGE = 3, FILTER_PAETH = 4};

//For all filters, the bytes "to the left of" the first pixel in a scanline must be treated as being zero. For filters that refer to the prior scanline, the entire prior scanline must be treated as being zeroes for the first scanline of an image (or of a pass of an interlaced image). 

void processIDATChunk (char* buffer, uintmax_t start, uint32_t chunk_length, uintmax_t buffer_size, IHDR ihdr, const uint32_t bytes_per_pixel) {
    // skipping the first two bytes of zlib header data
    std::vector<uint8_t> read = inflate::decompressZlib(buffer + start, chunk_length);
    std::vector<uint8_t> output;
    const uint32_t scanline_length = (ihdr.width * bytes_per_pixel) + 1;
    // now process the data!
    size_t row_count = 0;
    for (size_t i = 0; i < read.size(); row_count++) {
        // switching on scanline filter type
        switch (read[i]) {
            case FILTER_NONE:
                i++;
                for (size_t j = 0; j < scanline_length - 1; j++, i++) {
                    output.push_back(read[i]);
                }
            break;
            case FILTER_SUB:
                i++;
                for (size_t j = 0; j < scanline_length - 1; j++, i++) {
                    if (j < bytes_per_pixel) {
                        output.push_back(read[i]);
                    } else {
                        output.push_back(read[i] + output[output.size() - 1]);
                    }
                }
            break;
            case FILTER_UP:
                i++;
                for (size_t j = 0; j < scanline_length - 1; j++, i++) {
                    if (row_count == 0) {
                        output.push_back(read[i]);
                    } else {
                        output.push_back(read[i] - output[output.size() - ihdr.width*bytes_per_pixel]);
                    }
                }
            break;
            case FILTER_AVERAGE:
                i++;
                for (size_t j = 0; j < scanline_length - 1; j++, i++) {
                    if (row_count == 0) {
                        if (j == 0) {
                            output.push_back(read[i]);
                        } else {
                            output.push_back(read[i] - floor((double)read[i - 1] / 2));
                        }
                    } else {
                        output.push_back(read[i] - floor(((double)read[i - 1] + read[i - scanline_length]) / 2));
                    }
                }
            break;
            case FILTER_PAETH:
                i++;
                for (size_t j = 0; j < scanline_length - 1; j++, i++) {
                    
                }
            break;
        }
    }
    for (auto& i : output) {
        
    }
}

// https://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html
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
    if (val != PNG_SIGNATURE_FIRST_FOUR) {
        delete[] buffer;
        return nullptr; // missing first four bytes
    }
    buffer_ptr++;
    val = *buffer_ptr;
    if (val != PNG_SIGNATURE_SECOND_FOUR) {
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
    if (val != PNG_IHDR_TAG) {
        return nullptr; // IHDR tag is wrong
    }
    buffer_ptr++;
    IHDR ihdr;
    readInto((char*)&ihdr, buffer, file_size, 16, 13);
    ihdr.width = FLIP_ENDIAN_32(ihdr.width);
    ihdr.height = FLIP_ENDIAN_32(ihdr.height);
    IMG img;
    bool pallete = false;
    uint32_t bytes_per_pixel = 0;
    switch (ihdr.color_type) {
        case 3:
        pallete = true;
        case 0:
        bytes_per_pixel = (ihdr.bit_depth < 8) ? 8 : ihdr.bit_depth;
        break;
        case 2:
        bytes_per_pixel = (ihdr.bit_depth * 3) / 8; // pixel is rgb triple
        break;
        case 4:
        bytes_per_pixel = (ihdr.bit_depth * 2) / 8; // grayscale plus alpha channel
        break;
        case 6:
        bytes_per_pixel = (ihdr.bit_depth * 4) / 8; // pixel is rgba
        break;
    }
    img = imageloader::createBlank(ihdr.width, ihdr.height, bytes_per_pixel);
    // process the actual chunks now!
    uintmax_t i = 33; // skipping crc at end of IHDR chunk
    while (i < file_size) {
        length = READ_AS_UINT32(buffer + i);
        length = FLIP_ENDIAN_32(length);
        i += 4;
        std::string cc = { buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]};
        val = READ_AS_UINT32(buffer + i);
        i += 4;
        // the chunk type
        switch (val) {
            case PNG_PLTE_TAG:
            break;
            case PNG_IDAT_TAG:
                processIDATChunk(buffer, i, length, file_size, ihdr, bytes_per_pixel);
            break;
        }
        // 4 extra byte for crc
        i += length + 4;
    }
    delete[] buffer;
    return img;
}
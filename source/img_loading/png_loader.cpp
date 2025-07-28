#include "image_loader.hpp"
#include "inflate.hpp"
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <new>


#define PNG_SIGNATURE_FIRST_FOUR 0x474E5089
#define PNG_SIGNATURE_SECOND_FOUR 0x0A1A0A0D
#define PNG_IHDR_TAG 0x52444849
#define PNG_PLTE_TAG 0x45544C50
#define PNG_IDAT_TAG 0x54414449
#define PNG_IEND_TAG 0x444E4549

#define FLIP_ENDIAN_32(x) ( \
    (((x) >> 24) & 0x000000FF) | \
    (((x) >> 8)  & 0x0000FF00) | \
    (((x) << 8)  & 0x00FF0000) | \
    (((x) << 24) & 0xFF000000) )

#define READ_AS_UINT32(x) *((uint32_t*)(x))
#define READ_24_BITS(x) (*((uint8_t*)x)) | ((*((uint8_t*)x + 1)) << 8) | ((*((uint8_t*)x + 2)) << 16)

#define WRITE_24_BITS(x, n) (((uint8_t*)x)[0] = (n & 0xff)); (((uint8_t*)x)[1] = (n & 0xff00)); (((uint8_t*)x)[2] = (n & 0xff0000));

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

uint8_t PaethPredictor (uint8_t a, uint8_t b, uint8_t c) {
    int p = (a + b) - c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);
    if (pa <= pb && pa <= pc) { return a; }
    else if (pb <= pc) { return b; }
    return c;
}

enum FilterType {FILTER_NONE = 0, FILTER_SUB = 1, FILTER_UP = 2, FILTER_AVERAGE = 3, FILTER_PAETH = 4};

//For all filters, the bytes "to the left of" the first pixel in a scanline must be treated as being zero. For filters that refer to the prior scanline, the entire prior scanline must be treated as being zeroes for the first scanline of an image (or of a pass of an interlaced image). 

std::vector<uint8_t> processIDATChunk (char* buffer, uintmax_t start, uint32_t chunk_length, uintmax_t buffer_size, IHDR ihdr, const uint32_t bytes_per_pixel) {
    // skipping the first two bytes of zlib header data
    std::vector<uint8_t> read = inflate::decompressZlib(buffer + start, chunk_length);
    std::vector<uint8_t> output;
    const uint32_t scanline_length = (ihdr.width * bytes_per_pixel);
    // now process the data!
    size_t row_count = 0;
    for (size_t i = 0; i < read.size(); row_count++) {
        size_t current_line_start = output.size();
        size_t prev_line_start = (row_count > 0) ? current_line_start - scanline_length : 0;
        
        // switching on scanline filter type
        switch (read[i]) {
            case FILTER_NONE:
                i++;
                for (size_t j = 0; j < scanline_length; j++, i++) {
                    output.push_back(read[i]);
                }
            break;
            case FILTER_SUB:
                i++;
                for (size_t j = 0; j < scanline_length; j++, i++) {
                    uint8_t left = (j >= bytes_per_pixel) ? output[current_line_start + j - bytes_per_pixel] : 0;
                    output.push_back(read[i] + left);
                }
            break;
            case FILTER_UP:
                i++;
                for (size_t j = 0; j < scanline_length; j++, i++) {
                    uint8_t up = (row_count > 0) ? output[prev_line_start + j] : 0;
                    output.push_back(read[i] + up);
                }
            break;
            case FILTER_AVERAGE:
                i++;
                for (size_t j = 0; j < scanline_length; j++, i++) {
                    uint8_t left = (j >= bytes_per_pixel) ? output[current_line_start + j - bytes_per_pixel] : 0;
                    uint8_t upper = (row_count > 0) ? output[prev_line_start + j] : 0;
                    uint8_t avg  = (left + upper) / 2;

                    output.push_back(read[i] + avg);
                }
            break;
            case FILTER_PAETH:
                i++;
                for (size_t j = 0; j < scanline_length; j++, i++) {
                    uint8_t left = (j >= bytes_per_pixel) ? output[current_line_start + j - bytes_per_pixel] : 0;
                    uint8_t upper = (row_count > 0) ? output[prev_line_start + j] : 0;
                    uint8_t upper_left = (row_count > 0 && j >= bytes_per_pixel) ? output[prev_line_start + j - bytes_per_pixel] : 0;
                    output.push_back(read[i] + PaethPredictor(left, upper, upper_left));
                }
            break;
        }
    }

    return output;
}


class BitReader {
private:
    uint8_t* data;
public:
    uint32_t offset;
    uint8_t bit_offset;
    BitReader(void* data) {
        this->data = (uint8_t*)data;
        offset = 0;
        bit_offset = 0;
    }
    // copy constructor
    BitReader(const BitReader& b) {
        this->data = b.data;
        this->offset = b.offset;
        this->bit_offset = b.bit_offset;
    } 
    // move constructor
    BitReader(const BitReader&& b) {
        this->data = b.data;
        this->offset = b.offset;
        this->bit_offset = b.bit_offset;
    } 
    // assignment operators
    BitReader& operator=(const BitReader& b) {
        return *this = BitReader(b);
    }
    BitReader& operator=(const BitReader&& b) noexcept {
        return *this = BitReader(b);
    }
    uint32_t readBits (uint8_t bits) {
        uint32_t val = 0;
        uint32_t total_bits = 0;
        for (int32_t i = bits; i > 0;) {
            uint32_t remaining = 8 - bit_offset;
            uint32_t to_read  = ((i) < (int32_t)remaining) ? i : remaining;
            uint32_t mask = ((1u << to_read) - 1);
            uint32_t chunk = (data[offset] >> bit_offset) & mask; // mask created here and grab data same line
            val |= (chunk << total_bits); // move captured data to correct spot in val
            bit_offset += to_read ;
            total_bits += to_read ;
            i -= to_read ;
            if (bit_offset > 7) {
                offset++;
                bit_offset = 0;
            }
        }
        return val;
    }
};

// https://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html
// https://www.w3.org/TR/png-3/#abstract

//issue with color type 2 could be the byte alignment?
//data seems to be correct
//maybe write a test to test the pixel data

IMG imageloader::loadPNG(std::string path, unsigned int w, unsigned int h) {
    // open file
    std::ifstream file;
    file.open(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open png " + path);
    }
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
    uint32_t bytes_per_pixel = 0;
    switch (ihdr.color_type) {
        case 3:
            bytes_per_pixel = 3;
        case 0:
            bytes_per_pixel = (ihdr.bit_depth < 8) ? 32 : (ihdr.bit_depth / 8); // set to 32 as signal to idat processor??
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
    std::vector<uint8_t> idat;
    std::vector<uint32_t> pallete;
    // process the actual chunks now!
    for (uintmax_t i = 33; i < file_size; ) {
        length = READ_AS_UINT32(buffer + i);
        length = FLIP_ENDIAN_32(length);
        i += 4;
        std::string cc = { buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]};
        // std::cout << cc << "\n";
        val = READ_AS_UINT32(buffer + i);
        i += 4;
        // the chunk type
        switch (val) {
            case PNG_PLTE_TAG:
                for (size_t j = 0; j < length; j+=3) {
                    pallete.push_back(READ_24_BITS(buffer + i + j));
                }
            break;
            case PNG_IDAT_TAG:
            {
                std::vector<uint8_t> dat = processIDATChunk(buffer, i, length, file_size, ihdr, bytes_per_pixel);
                for (auto& i : dat) {
                    idat.push_back(i);
                }
            }
            break;
            case PNG_IEND_TAG:
                i = file_size;
            break;
        }
        // 4 extra byte for crc
        i += length + 4;
    }
    glGenTextures(1, &img->tex);
    glActiveTexture_g(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img->tex);
    glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // now create gl image
    switch (ihdr.color_type) {
        case 3:
        // process the data and set the pixels based on pallete
            if (pallete.size() == 0) {
                throw std::runtime_error("Failed to parse pallete for pallete based color png!");
            }
            {
                BitReader br(idat.data());
                for(size_t i = 0; br.offset < idat.size() && i < img->size; i+=3) {
                    uint32_t index = br.readBits(ihdr.bit_depth);
                    if (index > pallete.size() - 1) {
                        throw std::runtime_error("Read pallete index outside of pallete bounds!");
                    }
                    uint32_t color = pallete[index];
                    WRITE_24_BITS(img->data + i, color);
                }
                glTextureStorage2D_g(img->tex, 1, GL_RGB8, img->w, img->h);
                glTextureSubImage2D_g(img->tex, 0, 0, 0, img->w, img->h, GL_RGB, GL_UNSIGNED_BYTE, img->data);
            }
        break;
        case 0:
        // figure out how to deal with less than 8 bit color here
        break;
        case 2:
            // rgb triple can just throw into gl texture
            {   
                for (size_t i = 0; i < idat.size(); i++) {
                    img->data[i] = idat[i];
                }
                const GLint format = (ihdr.bit_depth == 8) ? GL_RGB8 : GL_RGB16;
                const GLenum type = (ihdr.bit_depth == 8) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
                img->format = format;
                img->type = type;
                glTextureStorage2D_g(img->tex, 1, format, img->w, img->h);
                glTextureSubImage2D_g(img->tex, 0, 0, 0, img->w, img->h, GL_RGB, type, img->data);
            }
        break;
        case 4:
            // grayscale with alpha
            {
                for (size_t i = 0; i < idat.size(); i++) {
                    img->data[i] = idat[i];
                }
                const GLint format = (ihdr.bit_depth == 8) ? GL_R8 : GL_R16;
                const GLenum type = (ihdr.bit_depth == 8) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
                img->format = format;
                img->type = type;
                glTextureStorage2D_g(img->tex, 1, format, img->w, img->h);
                glTextureSubImage2D_g(img->tex, 0, 0, 0, img->w, img->h, GL_R, type, img->data);
            }
        break;
        case 6:
            // rgb triple with alpha
            {
                for (size_t i = 0; i < idat.size(); i++) {
                    img->data[i] = idat[i];
                }
                const GLint format = (ihdr.bit_depth == 8) ? GL_RGBA8 : GL_RGBA16;
                const GLenum type = (ihdr.bit_depth == 8) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
                img->format = format;
                img->type = type;
                glTextureStorage2D_g(img->tex, 1, format, img->w, img->h);
                glTextureSubImage2D_g(img->tex, 0, 0, 0, img->w, img->h, GL_RGBA, type, img->data);
            }
        break;
    }
    delete[] buffer;

    glGenerateMipmap_g(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
    return img;
}
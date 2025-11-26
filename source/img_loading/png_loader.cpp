#include "image_loader.hpp"
#include "inflate.hpp"
#include <GL/gl.h>
#if defined(__unix__)
#include <GL/glext.h>
#endif
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <new>
#include <stdexcept>


#define PNG_SIGNATURE_FIRST_FOUR 0x474E5089
#define PNG_SIGNATURE_SECOND_FOUR 0x0A1A0A0D
#define PNG_IHDR_TAG 0x52444849
#define PNG_PLTE_TAG 0x45544C50
#define PNG_IDAT_TAG 0x54414449
#define PNG_IEND_TAG 0x444E4549
#define PNG_SRGB_TAG 0x52444849
#define PNG_CHRM_TAG 0x414d4167
#define PNG_GAMA_TAG 0x42475273
#define PNG_ICCP_TAG 0x0
#define PNG_TRNS_TAG 0x0

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

PREVENT_PACKING_STRUCT CHRM {
    uint32_t whitePointX;
    uint32_t whitePointY;
    uint32_t redX;
    uint32_t redY;
    uint32_t greenX;
    uint32_t greenY;
    uint32_t blueX;
    uint32_t blueY;
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

std::vector<uint8_t> processIDATChunk (std::vector<uint8_t> buffer, IHDR ihdr, const uint32_t bytes_per_pixel) {
    std::vector<uint8_t> read = inflate::decompressZlib(buffer.data(), buffer.size());
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

// can load all color types now
// not supporting these cause I'm lazy
// chunks yet to be added
//  -tRNS
//  -iCCP
//  -sRGB
//  -cHRM

gore::IMG gore::imageloader::loadPNG(std::string path) {
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
    if (ihdr.interlace) {
        throw std::runtime_error("Tried to load interlaced PNG image! Unsupported!");
    }
    IMG img;
    uint32_t bytes_per_pixel = 0;
    uint32_t valRange = 255;
    switch (ihdr.color_type) {
        case 3:
            bytes_per_pixel = 3;
        break;
        case 0:
            bytes_per_pixel = (ihdr.bit_depth <= 8) ? 1 : 2; // set to 32 as signal to idat processor??
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
    if (ihdr.color_type != 3) {
        switch (ihdr.bit_depth) {
            case 1:
                valRange = 1;
            break;
            case 2:
                valRange = 3;
            break;
            case 4:
                valRange = 15;
            break;
            case 8:
                valRange = 255;
            break;
            case 16:
                valRange = 65535;
            break;
        }
    }

    img = imageloader::createBlank(ihdr.width, ihdr.height, bytes_per_pixel);
    std::vector<uint8_t> idat;
    std::vector<uint32_t> pallete;
    int32_t sRgb = -1;
    uint32_t gamma = 0;
    bool gammaUsed = false;
    bool chrmUsed = false;
    CHRM chrm;
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
                    uint8_t fb = buffer[i + j];
                    uint8_t sb = buffer[i + j + 1];
                    uint8_t tb = buffer[i + j + 2];
                    pallete.push_back((fb << 16) | (sb << 8) | tb);
                }
            break;
            case PNG_IDAT_TAG:
            {
                for (size_t j = 0; j < length; j++) {
                    idat.push_back(*(buffer + i + j));
                }
            }
            break;
            case PNG_SRGB_TAG:
                sRgb = buffer[i];
            break;
            case PNG_GAMA_TAG:
                gamma = READ_AS_UINT32(buffer + i);
                gammaUsed = true;
            break;
            case PNG_CHRM_TAG:
                readInto((char*)&chrm, buffer, file_size, i, sizeof(CHRM));
                chrm.whitePointX = FLIP_ENDIAN_32(chrm.whitePointX);
                chrm.whitePointY = FLIP_ENDIAN_32(chrm.whitePointY);
                chrm.redX = FLIP_ENDIAN_32(chrm.redX);
                chrm.redY = FLIP_ENDIAN_32(chrm.redY);
                chrm.greenX = FLIP_ENDIAN_32(chrm.greenX);
                chrm.greenY = FLIP_ENDIAN_32(chrm.greenY);
                chrm.blueX = FLIP_ENDIAN_32(chrm.blueX);
                chrm.blueY = FLIP_ENDIAN_32(chrm.blueY);
            break;
            case PNG_IEND_TAG:
                i = file_size;
            break;
        }
        // 4 extra byte for crc
        i += length + 4;
    }
    idat = processIDATChunk(idat, ihdr, (ihdr.color_type == 3) ? 1 : bytes_per_pixel);
    glGenTextures(1, &img->tex);
    glActiveTexture_g(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img->tex);
    glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
                    img->data[i] = (color >> 16) & 0xff;
                    img->data[i + 1] = (color >> 8) & 0xff;
                    img->data[i + 2] = color & 0xff;
                }
                img->format = GL_RGB8;
                img->type = GL_UNSIGNED_BYTE;
                glTextureStorage2D_g(img->tex, 1, GL_RGB8, img->w, img->h);
                glTextureSubImage2D_g(img->tex, 0, 0, 0, img->w, img->h, GL_RGB, GL_UNSIGNED_BYTE, img->data);
            }
        break;
        case 0:
        {
            BitReader br(idat.data());
            for(size_t i = 0; br.offset < idat.size() && i < img->size;i++)  {
                uint32_t val = br.readBits(ihdr.bit_depth);
                switch (ihdr.bit_depth) {
                    case 1:
                        img->data[i] = (val * 255);
                    break;
                    case 2:
                        img->data[i] = (val * 85);
                    break;
                    case 4:
                        img->data[i] = (val * 17);
                    break;
                    case 8:
                        img->data[i] = val;
                    break;
                    case 16:
                        img->data[i] = (val & 0xff);
                        img->data[i + 1] = (val >> 8);
                        i++;
                    break;
                }
            }
            const GLint format = (ihdr.bit_depth <= 8) ? GL_R8 : GL_R16;
            const GLenum type = (ihdr.bit_depth <= 8) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
            img->format = format;
            img->type = type;
            glTextureStorage2D_g(img->tex, 1, format, img->w, img->h);
            glTextureSubImage2D_g(img->tex, 0, 0, 0, img->w, img->h, GL_RED, type, img->data);
        }
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
                const GLint format = (ihdr.bit_depth == 8) ? GL_RG8 : GL_RG16;
                const GLenum type = (ihdr.bit_depth == 8) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
                img->format = format;
                img->type = type;
                glTextureStorage2D_g(img->tex, 1, format, img->w, img->h);
                glTextureSubImage2D_g(img->tex, 0, 0, 0, img->w, img->h, GL_RG, type, img->data);
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
    if (gammaUsed) {
        float gammaVal = gamma / 100000.0f;
        for (size_t i = 0; i < img->size; i++) {
            float normal = (float)img->data[i] / (float)valRange;
            float convert = std::pow(normal, (1.0/gammaVal));
            uint8_t out = (uint8_t)(convert * valRange);
            img->data[i] = out; 
        }
    }
    if (chrmUsed) {

    }
    glGenerateMipmap_g(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
    return img;
}
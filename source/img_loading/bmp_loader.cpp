#include "../image_loader.hpp"
/*
//https://docs.fileformat.com/image/bmp/
//https://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
//https://en.wikipedia.org/wiki/BMP_file_format
//https://www.fileformat.info/format/bmp/egff.htm
//https://medium.com/sysf/bits-to-bitmaps-a-simple-walkthrough-of-bmp-image-format-765dc6857393
//http://www.paulbourke.net/dataformats/bmp/
*/

#pragma pack(push, 1)
struct BITMAPHEADER{
    int16_t identifier;
    int32_t size;
    int16_t reserved_1;
    int16_t reserved_2;
    int32_t offset;
};
#pragma pack(pop)
//portable instead of using win32

#pragma pack(push, 1)
struct BITMAPINFOHEADERV1{
    int32_t header_size;
    int32_t width;
    int32_t height;
    int16_t color_planes;
    int16_t bitspp;
    //new values
    int32_t compression;
    int32_t image_size;
    int32_t hor_rez;
    int32_t ver_rez;
    int32_t color_pallete;
    int32_t important_colors;
};
//v2-v3 are undocumentated so not getting supported lmao
struct BITMAPINFOHEADERV4{
    int32_t header_size;
    int32_t width;
    int32_t height;
    int16_t color_planes;
    int16_t bitspp;
    int32_t compression;
    int32_t image_size;
    int32_t hor_rez;
    int32_t ver_rez;
    int32_t color_pallete;
    int32_t important_colors;
    //new values
    uint32_t red_mask;    
	uint32_t green_mask;
	uint32_t blue_mask;
	uint32_t alpha_mask;
	uint32_t cs_type;
	int32_t  red_x;
	int32_t  red_y;
	int32_t  red_z;
	int32_t  green_x;
	int32_t  green_y;
	int32_t  green_z;
	int32_t  blue_x;
	int32_t  blue_y;
	int32_t  blue_z;
	uint32_t gamma_red;
	uint32_t gamma_green;
	uint32_t gamma_blue;
};

struct BITMAPINFOHEADERV5{
    int32_t header_size;
    int32_t width;
    int32_t height;
    int16_t color_planes;
    int16_t bitspp;
    int32_t compression;
    int32_t image_size;
    int32_t hor_rez;
    int32_t ver_rez;
    int32_t color_pallete;
    int32_t important_colors;
    uint32_t red_mask;    
	uint32_t green_mask;
	uint32_t blue_mask;
	uint32_t alpha_mask;
	uint32_t cs_type;
	int32_t  red_x;
	int32_t  red_y;
	int32_t  red_z;
	int32_t  green_x;
	int32_t  green_y;
	int32_t  green_z;
	int32_t  blue_x;
	int32_t  blue_y;
	int32_t  blue_z;
	uint32_t gamma_red;
	uint32_t gamma_green;
	uint32_t gamma_blue;
    //new values
    uint32_t intent;
    uint32_t profile_data;
    uint32_t profile_size;
    uint32_t reserved;
};

struct FourBytePallete{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
};

#pragma pack(pop)
//supports bitmapinfoheader

uint8_t loopUntilOnBit(uint32_t value){
    uint8_t pos = 0;
    uint8_t last_pos = 0;
    for(;pos < 32; pos++){
        uint8_t v = (value >> pos) & 1;
        if(v == 1){
            last_pos = pos;
            break;
        }
    }
    return last_pos;
}
uint8_t countbits(uint32_t value){
    uint8_t count = 0;
    for(uint8_t i = 0; i < 32; i++){
        uint8_t v = (value >> i) & 1;
        if(v == 1){
            count++;
        }
    }
    return count;
}


uint8_t convertRange(uint8_t val, uint8_t old_range){
    uint8_t new_range = (255);
    uint8_t new_val = (((val) * new_range) / old_range);
    return new_val;
}
//figure out how to combine them right
    //loop each mask until we hit first on bit and then use that position to determine how far to shift value over?
    //extract indiviual bits with << till bit and then &1
    //change range of colors to match new 8bit color version
    //it's reading rgba from right to left in the binary

//fix alpha mask being empty causing all colors not to render
uint8_t* mask16Bit(uint8_t* data, size_t size, BITMAPINFOHEADERV5 dib_header){
    uint8_t alpha_bit = loopUntilOnBit(dib_header.alpha_mask);
    uint8_t red_bit = loopUntilOnBit(dib_header.red_mask);
    uint8_t green_bit = loopUntilOnBit(dib_header.green_mask);
    uint8_t blue_bit = loopUntilOnBit(dib_header.blue_mask);

    uint8_t alpha_bit_count = countbits(dib_header.alpha_mask);
    uint8_t red_bit_count = countbits(dib_header.red_mask);
    uint8_t blue_bit_count = countbits(dib_header.blue_mask);
    uint8_t green_bit_count = countbits(dib_header.green_mask);

    uint8_t alpha_range = (uint8_t)std::pow(2, alpha_bit_count) - 1;
    uint8_t red_range = (uint8_t)std::pow(2, red_bit_count) - 1;
    uint8_t green_range = (uint8_t)std::pow(2, green_bit_count) - 1;
    uint8_t blue_range = (uint8_t)std::pow(2, blue_bit_count) - 1;

    uint16_t* ut = reinterpret_cast<uint16_t*>(data);
    uint8_t* new_data = new uint8_t[size * 2];
    size_t rs = size/2;
    for(size_t i = 0; i < rs; i++){
        uint32_t red = ut[i] & dib_header.red_mask;
        uint32_t green = ut[i] & dib_header.green_mask;
        uint32_t blue = ut[i] & dib_header.blue_mask;
        uint32_t alpha = ut[i] & dib_header.alpha_mask;
        uint8_t red_8 = (red >> red_bit);
        uint8_t green_8 = (green >> green_bit);
        uint8_t blue_8 = (blue >> blue_bit);
        uint8_t alpha_8 = (alpha >> alpha_bit);
        red_8 = convertRange(red_8, red_range);
        green_8 = convertRange(green_8, green_range);
        blue_8 = convertRange(blue_8, blue_range);
        (dib_header.alpha_mask == 0) ? alpha_8 = 255 : alpha_8 = convertRange(alpha_8, alpha_range);
        uint32_t full_color = (((uint32_t)red_8)) | (((uint32_t)green_8) << (8)) | (((uint32_t)blue_8) << (16)) | ((uint32_t)(alpha_8) << 24);

        uint32_t* t = (uint32_t*)new_data;
        t[i] = full_color;
    }
    delete data;
    return new_data;
}

//mask applied to rgba files
//cant edit const char array
uint8_t* mask32Bit(uint8_t* data, size_t size, BITMAPINFOHEADERV5 dib_header){
    uint32_t* ut = reinterpret_cast<uint32_t*>(data);
    uint8_t* new_data = new uint8_t[size];
    size_t rs = size/4;
    uint8_t alpha_bit = loopUntilOnBit(dib_header.alpha_mask);
    uint8_t red_bit = loopUntilOnBit(dib_header.red_mask);
    uint8_t green_bit = loopUntilOnBit(dib_header.green_mask);
    uint8_t blue_bit = loopUntilOnBit(dib_header.blue_mask);

    uint8_t alpha_bit_count = countbits(dib_header.alpha_mask);
    uint8_t red_bit_count = countbits(dib_header.red_mask);
    uint8_t blue_bit_count = countbits(dib_header.blue_mask);
    uint8_t green_bit_count = countbits(dib_header.green_mask);

    uint8_t alpha_range = (uint8_t)std::pow(2, alpha_bit_count) - 1;
    uint8_t red_range = (uint8_t)std::pow(2, red_bit_count) - 1;
    uint8_t green_range = (uint8_t)std::pow(2, green_bit_count) - 1;
    uint8_t blue_range = (uint8_t)std::pow(2, blue_bit_count) - 1;


    for(size_t i = 0; i < rs; i++){
        uint32_t full_color = *ut;
        uint32_t red = ut[i] & dib_header.red_mask;
        uint32_t green = ut[i] & dib_header.green_mask;
        uint32_t blue = ut[i] & dib_header.blue_mask;
        uint32_t alpha = ut[i] & dib_header.alpha_mask;
        uint8_t red_8 = (red >> red_bit);
        uint8_t green_8 = (green >> green_bit);
        uint8_t blue_8 = (blue >> blue_bit);
        uint8_t alpha_8 = (alpha >> alpha_bit);
        red_8 = convertRange(red_8, red_range);
        green_8 = convertRange(green_8, green_range);
        blue_8 = convertRange(blue_8, blue_range);
        (dib_header.alpha_mask == 0) ? alpha_8 = 255 : alpha_8 = convertRange(alpha_8, alpha_range);
        full_color = (((uint32_t)red_8)) | (((uint32_t)green_8) << (8)) | (((uint32_t)blue_8) << (16)) | ((uint32_t)(alpha_8) << 24);
        uint32_t* t = (uint32_t*)new_data;
        t[i] = full_color;
    }
    delete data;
    return new_data;
}

uint8_t* parse8BitColor(uint8_t* data, size_t size, BITMAPINFOHEADERV5 dib_header, uint8_t* pallete) {
    uint32_t new_size = dib_header.width * dib_header.height * 3;
    uint8_t* new_data = new uint8_t[new_size];
    std::vector<uint32_t> color_pallete;
    //now read pallete
    uint32_t* pal = (uint32_t*)pallete;
    for(int32_t i = 0; i < dib_header.color_pallete; i++){
        color_pallete.push_back(*(pal + i)); 
    }
    //upscale to 8bit bgr
    for (uint32_t i = 0, j = 0; i < size && j < new_size; i++, j += 3) {
        uint8_t in = data[i];
        uint32_t full_color = color_pallete[in];
        uint32_t blue = full_color & (0xff);
        uint32_t green = full_color & (0xff << 8);
        uint32_t red = full_color & (0xff << 16);
        green = green >> 8;
        red = red >> 16;
        new_data[j] = (uint8_t)blue;
        new_data[j + 1] = (uint8_t)green;
        new_data[j + 2] = (uint8_t)red;

    }
    delete data;
    return new_data;
}

uint8_t extract4Bits(uint8_t t, bool front){
    return (front) ? (((t) & 128) >> 4) | (((t) & 64) >> 4) | (((t) & 32) >> 4) | (((t) & 16) >> 4) : ((t) & 8)  | ((t) & 4) | ((t) & 2) | ((t) & 1);
}


//reading the padding?, have to fix in here because can't drop half of bytes
uint8_t* parse4BitColor(uint8_t* data, size_t size, BITMAPINFOHEADERV5 dib_header, uint8_t* pallete) {
    uint32_t new_size = dib_header.width * dib_header.height * 3; //sized for bgr
    uint8_t new_padding = (dib_header.width * 3) % 4; //padding that opengl expects in bgr texture apparentely
    new_size += ((dib_header.height - 1) * new_padding); //padding per line
    uint8_t* new_data = new uint8_t[new_size];

    std::memset(new_data, 0, new_size); //init data to zeros

    std::vector<uint32_t> color_pallete;
    //now read pallete
    uint32_t* pal = (uint32_t*)pallete;
    for(int32_t i = 0; i < dib_header.color_pallete; i++){
        color_pallete.push_back(*(pal + i)); 
    }

    uint32_t byte_width  = dib_header.width / 2;
    uint32_t half_width = dib_header.width;
    int8_t padding = 4 - (byte_width % 4);

    //this took forever to figure out
    for(uint32_t i = 0, j = 0; i < size && j < new_size; i += byte_width + padding){ //add the width of the row in bytes + the padding in the file
        for(uint32_t w = 0; w < half_width && j < new_size; w++){
            uint8_t index = i + (w / 2);
            uint8_t t = data[index];
            t = extract4Bits(t, ((w) % 2 == 0)); //switch part of byte to extract based on location in row
            uint32_t full_color = color_pallete[t];
            uint32_t blue = full_color & (0xff);
            uint32_t green = (full_color >> 8) & (0xff);
            uint32_t red = (full_color >> 16) & (0xff);

            new_data[j++] = (uint8_t)blue;
            new_data[j++] = (uint8_t)green;
            new_data[j++] = (uint8_t)red;
        }
        j += new_padding; // the remaining padding byte opengl is expecting
    }
    delete data;
    return new_data;
}


uint8_t extract1Bit(uint8_t t, uint8_t n){
    uint8_t f = (t << n) & 128;
    return f >> 7;
}

uint8_t* parse1BitColor(uint8_t* data, size_t size, BITMAPINFOHEADERV5 dib_header, uint8_t* pallete){
    uint32_t new_size = dib_header.width * dib_header.height * 3; //sized for bgr
    uint8_t new_padding = (dib_header.width * 3) % 4; //padding that opengl expects in bgr texture apparentely
    new_size += ((dib_header.height - 1) * new_padding); //padding per line
    uint8_t* new_data = new uint8_t[new_size];

    std::memset(new_data, 0, new_size); //init data to zeros

    std::vector<uint32_t> color_pallete;
    //now read pallete
    uint32_t* pal = (uint32_t*)pallete;
    for(int32_t i = 0; i < dib_header.color_pallete; i++){
        color_pallete.push_back(*(pal + i)); 
    }

    uint32_t byte_width  = dib_header.width / 4;
    uint32_t bit_width = dib_header.width;
    uint32_t bit_size = dib_header.width * dib_header.height;
    uint32_t bit_count = 0;
    int8_t padding = 4 - (byte_width % 4);

    for(uint32_t i = 0, j = 0; i < size && j < new_size; i+=padding) { //add the padding
        //parse 1 byte
        for(uint32_t w = 0; w < bit_width; w++, bit_count++){
            if(bit_count == 8) {
                i++;
                bit_count = 0;
            }
            uint8_t index = (i + (bit_count / 8));
            uint8_t t = data[i];
            uint8_t f = (w % 8);
            t = extract1Bit(t, f);
            uint32_t full_color = color_pallete[t];
            uint32_t blue = full_color & (0xff);
            uint32_t green = (full_color >> 8) & (0xff);
            uint32_t red = (full_color >> 16) & (0xff);
            new_data[j++] = (uint8_t)blue;
            new_data[j++] = (uint8_t)green;
            new_data[j++] = (uint8_t)red;
        }
        //skip remainder of this byte
        i++;
        bit_count = 0;
        j += new_padding;
    }
    delete data;
    return new_data;
}

//compression
//0 - none
//1 - 8-bit RLE algorithm
//2 - 4-bit RLE algorithm
//3 - bitfields encoding (16 and 32 only allow this one outside of 0)

//support RLE algorithms

/*
The Windows BMP format supports a simple run-length encoded (RLE) compression scheme for compressing 4-bit and 8-bit bitmap data.
 Since this is a byte-wise RLE scheme, 1-, 16-, 24-, and 32-bit bitmaps cannot be compressed using it, 
 due to the typical lack of long runs of bytes with identical values in these types of data. 
*/

/*
00 00 end of scan line
00 01 end of bitmap data
00 02 xx yy run offset marker
*/


uint8_t* decode8RLE(uint8_t* data, size_t size, BITMAPINFOHEADERV5 dib_header){
    uint32_t n_size = dib_header.width * dib_header.height;
    uint8_t* new_data = new uint8_t[dib_header.width * dib_header.height]; //since this is the size in bytes
    std::memset(new_data, 0, n_size);

    uint32_t byte_width  = dib_header.width;
    uint8_t padding = 4 - (byte_width % 4);

    uint32_t n = 0;
    int32_t w = 0;
    uint32_t i = 0;
    for(i = 0; i < size && n < n_size;){
        uint8_t count = data[i++];
        uint8_t values = data[i++];
        if(count == 0){
            switch(values){
                case 0:
                    if(w > 0){
                        n += (dib_header.width - w);
                    }
                    w = 0;
                break;
                case 1:
                    i = size;
                break;
                case 2:
                    //run offset marker
                    w = (data[i]);
                    n += (data[i++]); //move forward in row
                    n += (data[i++] * byte_width); //move forward in height
                break;
                default:
                    for(uint8_t j = 0; j < values && n < n_size && i < size; j++, n++, i++){
                        new_data[n] = data[i];
                    }
                    uint8_t p = 4 - ((values) % 4);
                    i += p;
                break;
            }
        }else{
            for(uint8_t j = 0; j < count && n < n_size; j++, n++, w++){
                new_data[n] = values;
            }
        }
        if(w >= dib_header.width){
            w = 0;
            n += padding;
        }
    }

    delete data;
    return new_data;
}


//For example, an RLE4-encoded data stream of 07 48 would decode to seven pixels, alternating in value as 04 08 04 08 04 08 04. 
uint8_t* decode4RLE(uint8_t* data, size_t size, BITMAPINFOHEADERV5 dib_header){
    uint32_t n_size = (dib_header.width) * dib_header.height;
    uint8_t* new_data = new uint8_t[dib_header.width * dib_header.height]; //since this is the size in bytes
    std::memset(new_data, 0, n_size);
    uint32_t n = 0; //current index in new_data
    uint32_t byte_width  = dib_header.width / 2;
    uint8_t padding = 4 - (byte_width % 4);

    int32_t w = 0;
    uint32_t i = 0;

    for(i = 0; i < size && n < n_size;){
        uint8_t count = data[i++];
        uint8_t values = data[i++];
        if (count == 0) {
            switch(values){
                case 0:
                    //end of scan line
                    if(w != 0){
                        n += (dib_header.width - w) / 2; //I don't know if this works properly
                    }
                    w = 0;
                break;
                case 1:
                    //end of bitmap data
                    i = size;
                break;
                case 2:
                    //run offset marker
                    w = (data[i]);
                    n += (data[i++] / 2); //move forward in row
                    n += (data[i++] * byte_width); //move forward in height
                    
                break;
                default:
                    //read raw data
                    uint8_t mode = 1;
                    for (uint8_t j = 0; j < values && n < n_size; j++, mode++, w++) {
                        uint8_t value = extract4Bits(data[i], !(mode % 2 == 0));
                        value = !(mode % 2 == 0) ? (value << 4) : value;
                        new_data[n] |= value;
                        uint8_t t = new_data[n];
                        n += (mode % 2 == 0) ? 1 : 0;
                        i += (mode % 2 == 0) ? 1 : 0;
                    }
                    n += (mode % 2 == 0) ? 1 : 0; //move over if one 4 bit unit left
                    i += (mode % 2 == 0) ? 1 : 0; //move over if one 4 bit unit left
                    //move over padding
                    uint8_t p = 4 - ((values / 2) % 4);
                    i += p;
                break;
            }
        } else {
            uint8_t mode = 1;
            for (uint8_t j = 0; j < count && n < n_size; j++, w++, mode++) {
                uint8_t value = extract4Bits(values, !(mode % 2 == 0));
                value = !(mode % 2 == 0) ? (value << 4) : value;
                new_data[n] |= value;
                uint8_t t = new_data[n];
                n += (mode % 2 == 0) ? 1 : 0;
           }
        }
        if(w >= dib_header.width){
            n += padding;
            w = 0;
        }
    }
    delete data;
    return new_data;
}

//only supports 8 bit color masks currently
IMG imageloader::loadBMP(std::string path){
    std::ifstream file;
    file.open(path, std::ios::binary | std::ios::ate);
    std::vector<char> buffer;
    std::string str;
    //do it this way since some images aren't fully dumped into memory with old method
    if(file){
        std::streamsize size = file.tellg();
        buffer = std::vector<char>(size);
        file.seekg(0, std::ios::beg);
        if(file.read(buffer.data(), size)){
            for(auto& i : buffer){
                str.push_back(i);
            }
        }
    }else{
        return nullptr; //failed to open
    }
    file.close();
    if(str.size() < 14){
        return nullptr; //not even sized large enough to have a header
    }
    char* m = (char*)str.c_str(); //not supposed to cast const away but we are never editing it so I don't think matters
    BITMAPHEADER* bit = (BITMAPHEADER*)(m);
    BITMAPHEADER bitheader = *bit;
    bit++;
    //dib header
    BITMAPINFOHEADERV5* bdp = (BITMAPINFOHEADERV5*)bit;
    BITMAPINFOHEADERV5 dib_header = *bdp;
    uint8_t* pallete = ((uint8_t*)bdp) + dib_header.header_size;

    //creaing the image data
    IMG img = new g_img;
    img->bytes_per_pixel = dib_header.bitspp/8;

    //account for negative height numbers
    if(dib_header.height < 0){
        dib_header.height = dib_header.height - (dib_header.height * 2);
    }
    img->h = dib_header.height;
    img->w = dib_header.width;
    int32_t real_size = (dib_header.bitspp > 4) ? img->h * img->w * img->bytes_per_pixel : (img->h * img->w) / (8 / dib_header.bitspp);
    if(dib_header.header_size <= 40 || (dib_header.bitspp <= 8)){
        img->data = new uint8_t[dib_header.image_size]; //img->w for this version includes padding
        if(!img->data){
            return nullptr;
        }
        uint8_t* um = (uint8_t*)m + bitheader.offset;
        for(int32_t i = 0; i < dib_header.image_size; i++){
            img->data[i] = um[i];
        }
    }
    else{
        img->data = new uint8_t[real_size];
        if(!img->data){
            return nullptr;
        }
        uint8_t* um = (uint8_t*)m + bitheader.offset;

        //Each scan line must end on a 4-byte boundary, so one, two, or three bytes of padding may follow each scan line.
        int32_t left_over = (img->w * img->bytes_per_pixel) % 4;
        for(int32_t i = 0, j = 0, w = 0, dif = 0; i < dib_header.image_size && j < real_size; j++, w++, i++) {
                img->data[j] = um[i + dif];
                if(w == (img->w * img->bytes_per_pixel) - 1){
                    w = 0;
                    dif += left_over;
                }
        }
    }
    if(img->bytes_per_pixel <= 1){
        img->bytes_per_pixel = 3; //since we are going to upscale to bgr
    }
    //creating the gl texture
    glGenTextures(1, &img->tex);
	glActiveTexture_g(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img->tex);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    if(dib_header.compression == 2){
        img->data = decode4RLE(img->data, dib_header.image_size, dib_header);
        dib_header.image_size = dib_header.width * dib_header.height;
    }else if(dib_header.compression == 1){
        img->data = decode8RLE(img->data, dib_header.image_size, dib_header);
        dib_header.image_size = dib_header.width * dib_header.height;
    }
    //need switch here to determine the type of pixel to load
    switch(dib_header.bitspp){
        case 1:
            img->data = parse1BitColor(img->data, dib_header.image_size, dib_header, pallete);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img->w, img->h, 0, GL_BGR, GL_UNSIGNED_BYTE, img->data); //done
        break;
        case 4:

            img->data = parse4BitColor(img->data, dib_header.image_size, dib_header, pallete);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img->w, img->h, 0, GL_BGR, GL_UNSIGNED_BYTE, img->data); //done
        break;
        case 8:
            img->data = parse8BitColor(img->data, real_size, dib_header, pallete);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img->w, img->h, 0, GL_BGR, GL_UNSIGNED_BYTE, img->data); //done
        break;
        case 16:
            if(dib_header.compression == 3){
                img->data = mask16Bit(img->data, real_size, dib_header);
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img->w, img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->data); //done
        break;
        case 24:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img->w, img->h, 0, GL_BGR, GL_UNSIGNED_BYTE, img->data); //done
        break;
        case 32:
            if(dib_header.compression == 3){
                img->data = mask32Bit(img->data, real_size, dib_header);
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img->w, img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->data); //done
        break;
    }
	glGenerateMipmap_g(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
    return img;
}
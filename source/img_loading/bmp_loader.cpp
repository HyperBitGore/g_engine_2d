#include "../image_loader.hpp"
/*
//https://docs.fileformat.com/image/bmp/
//https://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
//https://en.wikipedia.org/wiki/BMP_file_format
//https://www.fileformat.info/format/bmp/egff.htm
//https://medium.com/sysf/bits-to-bitmaps-a-simple-walkthrough-of-bmp-image-format-765dc6857393
//http://www.paulbourke.net/dataformats/bmp/
*/

#pragma pack(push, 1);
struct BITMAPHEADER{
    int16_t identifier;
    int32_t size;
    int16_t reserved_1;
    int16_t reserved_2;
    int32_t offset;
};
#pragma pack(pop);
//portable instead of using win32

#pragma pack(push, 1);
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
    uint32_t alpha_mask;    
	uint32_t red_mask;
	uint32_t green_mask;
	uint32_t blue_mask;
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

#pragma pack(pop);
//supports bitmapinfoheader
//24 bit uncompressed with no color pallete

//flips it into regular RGBA8 image
void mask32Bit(uint8_t* data, size_t size, BITMAPINFOHEADERV5 dib_header){
    uint32_t* ut = reinterpret_cast<uint32_t*>(data);
    size_t rs = size/4;
    for(size_t i = 0; i < rs; i++){
        uint32_t n = *ut;
        n = (*ut) & (dib_header.alpha_mask | dib_header.red_mask | dib_header.green_mask | dib_header.blue_mask);
        *ut = n;
    }
}

//compression
//0 - none
//1 - 8-bit RLE algorithm
//2 - 4-bit RLE algorithm
//3 - bitfields encoding (16 and 32 only allow this one outside of 0)
//if 3 mask follows after header

//support color palletes for 1,4,8 bit
//support RLE algorithms
//support color masks

//figured out why test4 and test1 are broekn, seems like the whole file isn't getting read into memory

IMG imageloader::loadBMP(std::string path){
    std::ifstream f_stream;
    f_stream.open(path, std::ios::binary);
    std::stringstream ss;
    if(f_stream){
        std::string line;
        while(getline(f_stream, line)){
            ss << line + '\n';
        }
    }else{
        return nullptr; //failed to open
    }
    f_stream.close();
    std::string str(ss.str());
    str.pop_back(); //drop last newline which is unneeded
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


    //creaing the image data
    IMG img = new g_img;
    img->bytes_per_pixel = dib_header.bitspp/8;
        //acount for negative height numbers
    if(dib_header.height < 0){
        dib_header.height = dib_header.height - (dib_header.height * 2);
    }
    img->h = dib_header.height;
    img->w = dib_header.width;
    img->data = new uint8_t[dib_header.image_size];
    //dumping the pixel array into image
    if(img->data){
        uint8_t* um = (uint8_t*)m + bitheader.offset;
        for(int32_t i = 0; i < dib_header.image_size; i++){
            img->data[i] = um[i];
        }
        // std::memcpy(img->data, (uint8_t*)m + bitheader.offset, dib_header.image_size);
    }

    switch(dib_header.header_size){
        case 40://v1

        break;
        case 108: //v4
        
        break;
        case 124: //v5
        
        break;
    }

    //creating the gl texture
    glGenTextures(1, &img->tex);
	glActiveTexture_g(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img->tex);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //need switch here to determine the type of pixel to load
    switch(dib_header.bitspp){
        case 1:

        break;
        case 4:

        break;
        case 8:

        break;
        case 16:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->w, img->h, 0, GL_RGB, GL_UNSIGNED_BYTE, img->data); //not done
        break;
        case 24:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img->w, img->h, 0, GL_BGR, GL_UNSIGNED_BYTE, img->data); //done
        break;
        case 32:
            if(dib_header.compression == 3){
                mask32Bit(img->data, dib_header.image_size, dib_header);
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, img->data); //done
        break;
    }
	glGenerateMipmap_g(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
    return img;
}
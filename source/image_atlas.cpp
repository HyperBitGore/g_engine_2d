#include "image_loader.h"

int imageHash(IMG img) {
		return img->data[0] % 256;
	}

//need to create an image 
ImageAtlas::ImageAtlas(int w, int h, int bytes_per_pixel) {
    images.setHashFunction(imageHash);
    img = new g_img;
    img->h = h;
    img->w = w;
    img->bytes_per_pixel = bytes_per_pixel;
    img->data = (unsigned char*)std::malloc((img->w * img->bytes_per_pixel) * img->h); //pixel is four bytes so w*4 is the stride
    std::memset(img->data, 0, (img->w * img->bytes_per_pixel) * img->h);
}
//have to make sure the n_img is the same format as the atlas
void ImageAtlas::addImage(IMG n_img) {
    if (n_img->bytes_per_pixel != img->bytes_per_pixel) {
        return;
    }
    int c_x = cur_x;
    int c_y = cur_y;
    if (cur_x + n_img->w + 1 >= img->w) {
        cur_y += n_img->h;
        cur_x = 0;
    }
    else {
        cur_x += n_img->w + 1;
    }
    if ((size_t)cur_y > img->h) {
        img->data = (unsigned char*)std::realloc(img->data, (img->w * 4) * (img->h + 200));
    }

    
    for (size_t y = 0; y < n_img->h; y++) {
        for (size_t x = 0; x < n_img->w; x++) {
            uint32_t col = (uint32_t)imageloader::getPixel(n_img, x, y, 4);
            imageloader::setPixel(img, c_x + x, c_y + y, col, 4);
        }
    }
    

    images.insert(n_img, { c_x, c_y });
}

Point ImageAtlas::getImagePos(IMG img, bool normalize) {
    if(normalize){
        Point p = *images.get(img);
        return {(p.x / (int)img->w), (p.y / (int)img->h)};
    }
    return *images.get(img);
}

IMG ImageAtlas::getImg() {
    return img;
}
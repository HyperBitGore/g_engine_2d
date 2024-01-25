#include "image_loader.h"



//need to create an image 
ImageAtlas::ImageAtlas(int w, int h, int bytes_per_pixel) {
    //images.setHashFunction(imageHash);

    //std::cout << "Colliding: " << checkCollision({10, 10}, {20, 20}, {10, 20}, {10, 10}) << "\n";
    max_images = 256;
    buckets = new Memb[256];
    for(int i = 0; i < max_images; i++){
        buckets[i] = nullptr;
    }
    img = new g_img;
    img->h = h;
    img->w = w;
    img->bytes_per_pixel = bytes_per_pixel;
    img->data = (unsigned char*)std::malloc((img->w * img->bytes_per_pixel) * img->h); //pixel is four bytes so w*4 is the stride
    std::memset(img->data, 0, (img->w * img->bytes_per_pixel) * img->h);
}
ImageAtlas::~ImageAtlas(){
    for(int i = 0; i < 256; i++){
        Memb cur = buckets[i];
        while(cur != nullptr){
            Memb next = cur->next;
            delete cur;
            cur = next;
        }
    }
    delete[] buckets;
}

bool ImageAtlas::checkCollision(Point p1, Point dim1, Point p2, Point dim2){
    if(p1.x + dim1.x >= p2.x && p1.x <= p2.x +dim2.x && p1.y + dim1.y >= p2.y && p1.y <= p2.y + dim2.y){
        return true;
    }
    return false;
}
//bottom of p1
bool checkBottomSide(Point p1, Point dim1, Point p2, Point dim2){
    return p1.y + dim1.y >= p2.y;
}
//top of p1
bool checkTopSide(Point p1, Point dim1, Point p2, Point dim2){
    return p1.y <= p2.y + dim2.y; //top side of p1
}
//left side of p1
bool checkLeftSide(Point p1, Point dim1, Point p2, Point dim2){
    return p1.x <= p2.x + dim2.x;
}
//right side of p1
bool checkRightSide(Point p1, Point dim1, Point p2, Point dim2){
    return p1.x + dim1.x >= p2.x;
}

bool ImageAtlas::spotEmpty(Point p, Point dim){
    for(int i = 0; i < max_images; i++){
        Memb cur = buckets[i];
        while(cur != nullptr){
            if(checkCollision(p, dim, cur->point, cur->dimensions)){
                return false;
            }
            cur = cur->next;
        }
    }
    return true;
}

Point ImageAtlas::findEmpty(Point start, Point dim){
    for(int y = start.y; y < img->h; y++){
        for(int x = start.x; x < img->w; x++){
            if(spotEmpty({x, y}, dim)){
               return {x, y}; 
            }
        }
    }
    return {-1, -1};
}

//have to make sure the n_img is the same format as the atlas
void ImageAtlas::addImage(IMG n_img, std::string name) {
    if (n_img->bytes_per_pixel != img->bytes_per_pixel) {
        return;
    }
    Point c = {-1, -1};
    while(c.x == -1){
        c = findEmpty({0, 0}, {(int)n_img->w, (int)n_img->h});
        if(c.x == -1){
            img->data = (unsigned char*)std::realloc(img->data, (img->w * img->bytes_per_pixel) * (img->h + 200));
        }
    }
    /*if (cur_x + n_img->w + 1 >= img->w) {
        cur_y += n_img->h;
        cur_x = 0;
    }
    else {
        cur_x += n_img->w + 1;
    }
    */
    /*if ((size_t)cur_y > img->h) {
        img->data = (unsigned char*)std::realloc(img->data, (img->w * 4) * (img->h + 200));
    }*/

    
    for (size_t y = 0; y < n_img->h; y++) {
        for (size_t x = 0; x < n_img->w; x++) {
            uint32_t col = (uint32_t)imageloader::getPixel(n_img, x, y, img->bytes_per_pixel);
            imageloader::setPixel(img, c.x + x, c.y + y, col, 4);
        }
    }
    
    insert(name, n_img, {c.x, c.y});
}

void ImageAtlas::addImage(std::string path, unsigned int w, unsigned int h, std::string name){
    IMG img = imageloader::loadPNG(path, w, h);
    addImage(img, name);
}

Point ImageAtlas::getImagePos(std::string name, bool normalize) {
    Memb memb = get(name);
    if(memb == nullptr){
        return {-1, -1};
    }
    Point p = memb->point;
    if(normalize){
        return {(p.x / (int)img->w), (p.y / (int)img->h)};
    }
    return p;
}

IMG ImageAtlas::getImg() {
    return img;
}
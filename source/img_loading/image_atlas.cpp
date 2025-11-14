#include "image_loader.hpp"


ImageAtlas::ImageAtlas () {
    buckets = nullptr;
    img = nullptr;
    max_images = 0;
}

//need to create an image 
ImageAtlas::ImageAtlas(int w, int h, int bytes_per_pixel, uint32_t max_images) {
    //images.setHashFunction(imageHash);

    //std::cout << "Colliding: " << checkCollision({10, 10}, {20, 20}, {10, 20}, {10, 10}) << "\n";
    this->max_images = max_images;
    buckets = new Memb[max_images];
    for(int i = 0; i < max_images; i++){
        buckets[i] = nullptr;
    }
    img = imageloader::createBlank(w, h, bytes_per_pixel);
}

ImageAtlas::ImageAtlas (const ImageAtlas& atlas) {
    if (this->buckets) {
        for(int i = 0; i < max_images; i++){
            Memb cur = buckets[i];
            while(cur != nullptr){
                Memb next = cur->next;
                delete cur;
                cur = next;
            }
        }
        delete this->buckets;
    }
    this->buckets = new Memb[atlas.max_images];
    for(int i = 0; i < max_images; i++) {
        if (atlas.buckets[i]) {
            Memb m = new Member;
            m->name = atlas.buckets[i]->name;
            m->next = nullptr;
            m->p_and_d = atlas.buckets[i]->p_and_d;
            this->buckets[i] = m;
            Memb cur = atlas.buckets[i]->next;
            Memb last = m;
            while (cur != nullptr) {
                Memb m2 = new Member;
                m2->name = cur->name;
                m2->p_and_d = cur->p_and_d;
                last->next = m2;
                cur = cur->next;
            }
        }
        
    }
    this->img = imageloader::createBlank(atlas.img->w, atlas.img->h, atlas.img->bytes_per_pixel);
    if (atlas.img) {
        std::memcpy(img->data, atlas.img->data, img->size);
    }
    this->max_images = atlas.max_images;
}
ImageAtlas::ImageAtlas (ImageAtlas&& atlas) {
    this->buckets = atlas.buckets;
    this->img = atlas.img;
    this->max_images = atlas.max_images;
    atlas.buckets = nullptr;
    atlas.img = nullptr;
    atlas.max_images = 0;
}

ImageAtlas::~ImageAtlas(){
    if (buckets) {
        for(int i = 0; i < max_images; i++){
            Memb cur = buckets[i];
            while(cur != nullptr){
                Memb next = cur->next;
                delete cur;
                cur = next;
            }
        }
        delete[] buckets;
    }
}

ImageAtlas& ImageAtlas::operator=(const ImageAtlas& atlas) {
    if (this == &atlas || atlas.buckets == nullptr) {
        return *this;
    }
    ImageAtlas at(atlas);
    swap(at, *this);
    return *this;
}

ImageAtlas& ImageAtlas::operator=(ImageAtlas&& atlas) {
    if (this == &atlas || atlas.buckets == nullptr) {
        return *this;
    }
    swap(*this, atlas);
    return *this;
}

bool ImageAtlas::checkCollision(Point p1, Point dim1, Point p2, Point dim2){
    if(p1.x + dim1.x >= p2.x && p1.x <= p2.x +dim2.x && p1.y + dim1.y >= p2.y && p1.y <= p2.y + dim2.y){
        return true;
    }
    return false;
}

bool ImageAtlas::spotEmpty(Point p, Point dim){
    if (p.x + dim.x > this->img->w || p.y + dim.y > this->img->h || p.x >= this->img->w || p.y >= this->img->h) {
        return false;
    }
    for(int i = 0; i < max_images; i++){
        Memb cur = buckets[i];
        while(cur != nullptr){
            if(checkCollision(p, dim, {(int)cur->p_and_d.x, (int)cur->p_and_d.y}, {(int)cur->p_and_d.z + 1, (int)cur->p_and_d.w + 1})){
                return false;
            }
            cur = cur->next;
        }
    }
    return true;
}

// should do this
// only need to look up if y is fine, the x position should be set based on previous insert

//have to make sure the n_img is the same format as the atlas
void ImageAtlas::addImage(IMG n_img, std::string name) {
    if (n_img->bytes_per_pixel != img->bytes_per_pixel) {
        return;
    }
    while (!spotEmpty({(int)start_pos.x, (int)start_pos.y}, {(int)n_img->w, (int)n_img->h})) {
        start_pos.x += 2;
        if (start_pos.x + n_img->w > img->w) {
            start_pos.x = 0;
            start_pos.y += 2;
        }
        if (start_pos.y + n_img->h > img->h) {
            uint8_t* new_data = new uint8_t[(img->w * img->bytes_per_pixel) * (img->h + 200)];
            std::memcpy(new_data, img->data, img->size);
            img->size = (img->w * img->bytes_per_pixel) * (img->h + 200);
            img->h += 200;
            img->data = new_data;
        }
    }
    // probably convert this to memcpy
    for (size_t y = 0; y < n_img->h; y++) {
        //std::memcpy(img->data + ((int)start_pos.y * (img->bytes_per_pixel * (int)start_pos.x )), n_img + (y * (n_img->bytes_per_pixel)), y * (n_img->bytes_per_pixel));
        for (size_t x = 0; x < n_img->w; x++) {
            uint32_t col = (uint32_t)imageloader::getPixel(n_img, x, y, img->bytes_per_pixel);
            imageloader::setPixelRaw(img, start_pos.x + x, start_pos.y + y, col, 4);
        }
    }
    start_pos.x += n_img->w;
    insert(name, n_img, {(float)start_pos.x, (float)start_pos.y});
}

void ImageAtlas::addImage(std::string path, IMG_TYPE type, std::string name){
    IMG img;
    switch(type){
        case IMG_TYPE::BMP:
            img = imageloader::loadBMP(path);
        break;
        case IMG_TYPE::PNG:
            img = imageloader::loadPNG(path);
        break;
    }
    addImage(img, name);
}

vec4 ImageAtlas::getImagePos(std::string name, bool normalize) {
    Memb memb = get(name);
    if(memb == nullptr){
        return {-1.0f, -1.0f, -1.0f, -1.0f};
    }
    vec4 p = memb->p_and_d;
    if(normalize){
        return {(p.x / (float)img->w), (p.y / (float)img->h), p.z / (float)img->w, p.w / (float)img->h};
    }
    return p;
}

IMG ImageAtlas::getImg() {
    return img;
}
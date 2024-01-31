#pragma once
#include "shader.hpp"

struct g_img {
	unsigned char* data;
	GLuint tex;
	unsigned int w;
	unsigned int h;
	uint8_t bytes_per_pixel;
};
typedef g_img* IMG;
struct Point {
	int x;
	int y;
};

//store a bunch of images which you can stich together
//adding an image stiches it into the image
class ImageAtlas {
private:
	struct Member{
		//point and dimensions
		vec4 p_and_d;
		std::string name;
		Member* next;
	};
	typedef Member* Memb;
	Memb* buckets;
	int imageHash(std::string name) {
        int tot = 0;
        for(size_t i = 0; i < name.size(); i++){
            tot += name[i];
        }
		return tot % 256;
	}
	void insert(std::string name, IMG img, vec2 point){
		int hash = imageHash(name);
		Memb m = new Member;
		m->p_and_d = {point.x, point.y, (float)img->w, (float)img->h};
		m->name = name;
		m->next = nullptr;
		 if(buckets[hash] != nullptr){
			Memb cur = buckets[hash];
			while(cur->next != nullptr){
				cur = cur->next;
			}
			cur->next = m;
		 }else{
			buckets[hash] = m;
		 }

	}
	bool checkCollision(Point p1, Point dim1, Point p2, Point dim2);
	bool spotEmpty(Point p, Point dim);
	Point findEmpty(Point start, Point dim);

	Memb get(std::string name){
		int hash = imageHash(name);
		Memb cur = buckets[hash];
		while(cur != nullptr && cur->name.compare(name) != 0){
			cur = cur->next;
		}
		return cur;
	}
	//Gore::HashMap<Member, std::string> images;
	IMG img;
	unsigned int max_images;

public:
	ImageAtlas(int w, int h, int bytes_per_pixel);
	~ImageAtlas();
	void addImage(IMG img, std::string name);
	void addImage(std::string path, unsigned int w, unsigned int h, std::string name);
	vec4 getImagePos(std::string name, bool normalize = false);
	IMG getImg();
};

//rewrite image loader to be manageable
class imageloader{
	public:
	static IMG createBlank(GLuint w, GLuint h, GLuint bytes_per_pixel);
	static void createTexture(IMG img, GLenum internalformat, GLenum format);
	static IMG loadPNG(std::string path, unsigned int w, unsigned int h);
	static IMG loadBMP(std::string path,  unsigned int w, unsigned int h);
	static void setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	static void setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b);
	static void setPixel(IMG img, int x, int y, uint8_t r, uint8_t g);
	static void setPixel(IMG img, int x, int y, uint8_t r);
	//assumed the color components are 8 bits each
	static void setPixel(IMG img, int x, int y, uint32_t color, int bytes);

	//run after you've done all the editing of data you want to
	static void updateIMG(IMG img) {
		glBindTexture(GL_TEXTURE_2D, (GLuint)img->tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->w, img->h, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	static uint64_t getPixel(IMG img, int x, int y, int bytes);
};
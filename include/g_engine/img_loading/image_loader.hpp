#pragma once
#include "../util/shader.hpp"
#include <memory>
#if defined(__unix__)
#include <GL/gl.h>
#include <GL/glext.h>
#endif
namespace gore {
//convert all data allocation to new so we can delete it in destructor
struct g_img {
	unsigned char* data;
	GLuint tex;
	unsigned int w;
	unsigned int h;
	uint8_t bytes_per_pixel;
	GLenum format;
	GLenum type;
	GLenum internalformat;
	size_t size;
	std::string name;
};
typedef std::unique_ptr<g_img> IMG;
struct Point {
	int x;
	int y;
};

enum class IMG_TYPE {BMP, PNG};

//store a bunch of images which you can stich together
//adding an image stiches it into the image
class imageatlas {
private:
	struct Member{
		//point and dimensions
		vec4 p_and_d;
		std::string name;
		Member* next;
	};
	typedef Member* Memb;
	Memb* buckets = nullptr;
	int imageHash(std::string name) {
        int tot = 0;
        for(size_t i = 0; i < name.size(); i++){
            tot += name[i];
        }
		return tot % max_images;
	}
	void insert(std::string name, const IMG& img, vec2 point){
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
	IMG img = nullptr;
	uint32_t max_images = 0;
	vec2 start_pos = {0, 0};
	friend void swap (imageatlas& a, imageatlas& b) {
		std::swap(a.img, b.img);
		std::swap(a.buckets, b.buckets);
		std::swap(a.max_images, b.max_images);
	}
public:
	imageatlas();
	imageatlas(int w, int h, int bytes_per_pixel, uint32_t max_images);
	// copy
	imageatlas (const imageatlas& atlas);
	//move
	imageatlas (imageatlas&& atlas);
	~imageatlas();
	// operators
	imageatlas& operator=(const imageatlas& atlas);
	imageatlas& operator=(imageatlas&& atlas);
	void addImage(const IMG& img, std::string name);
	void addImage(std::string path, IMG_TYPE type, std::string name);
	vec2 getNextImagePos (uint32_t w, uint32_t h);
	void insert(std::string name, uint32_t w, uint32_t h, vec2 point){
		int hash = imageHash(name);
		Memb m = new Member;
		m->p_and_d = {point.x, point.y, (float)w, (float)h};
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
	vec4 getImagePos(std::string name, bool normalize = false);
	const IMG& getImg();
};

//rewrite image loader to be manageable
class imageloader{
	public:
	static IMG createBlank(GLuint w, GLuint h, GLuint bytes_per_pixel);
	static void createTexture(const IMG& img, GLenum internalformat, GLenum format, GLenum type);
	static IMG copyIMG (const IMG& img);
	static IMG loadPNG(std::string path);
	static IMG loadBMP(std::string path);
	static IMG convertIMGRGBA8(const IMG& img);
	static void setPixel(const IMG& img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	static void setPixel(const IMG& img, int x, int y, uint8_t r, uint8_t g, uint8_t b);
	static void setPixel(const IMG& img, int x, int y, uint8_t r, uint8_t g);
	static void setPixel(const IMG& img, int x, int y, uint8_t r);
	//assumed the color components are 8 bits each
	static void setPixelRaw(const IMG& img, int x, int y, uint32_t color, int bytes);

	//run after you've done all the editing of data you want to
	static void updateIMG(const IMG& img) {
		glBindTexture(GL_TEXTURE_2D, (GLuint)img->tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->w, img->h, img->format, img->type, img->data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	static uint64_t getPixel(const IMG& img, int x, int y, int bytes);
};
}
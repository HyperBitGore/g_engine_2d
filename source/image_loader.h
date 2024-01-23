#pragma once
#include "shader.h"

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
	Gore::HashMap<Point, IMG> images;
	IMG img;
	int cur_y = 0;
	int cur_x = 0;
public:
	ImageAtlas(int w, int h, int bytes_per_pixel);
	void addImage(IMG img);
	Point getImagePos(IMG img, bool normalize = false);
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
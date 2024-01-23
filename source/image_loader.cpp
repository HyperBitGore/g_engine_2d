#include "image_loader.h"

IMG imageloader::loadPNG(std::string path, unsigned int w, unsigned int h){
	IMG img = new g_img;
	unsigned error = lodepng_decode32_file((&(img->data)), &w, &h, path.c_str());
	img->w = w;
	img->h = h;
	img->bytes_per_pixel = 4;
	glGenTextures(1, &img->tex);
	glActiveTexture_g(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img->tex);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
	glGenerateMipmap_g(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	return img;
}

IMG imageloader::createBlank(GLuint w, GLuint h, GLuint bytes_per_pixel){
	IMG img = new g_img;
	img->h = h;
	img->w = w;
	img->bytes_per_pixel = bytes_per_pixel;
	img->data = (unsigned char*)std::malloc((w * bytes_per_pixel) * h); //pixel is four bytes so w*4 is the stride
	std::memset(img->data, 0, (w * bytes_per_pixel) * h);

	return img;
}

void imageloader::createTexture(IMG img, GLenum internalformat, GLenum format){
	glGenTextures(1, &img->tex);
	//glActiveTexture_g(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img->tex);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D_g(img->tex, 1, internalformat, img->w, img->h);
	glTexImage2D(img->tex, 0, 0, 0, img->w, img->h, format, GL_UNSIGNED_BYTE, img->data);
	glGenerateMipmap_g(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void imageloader::setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	size_t row = y * (img->w * 4);
	size_t col = x * 4;
	img->data[row + col] = r;
	img->data[row + col + 1] = g;
	img->data[row + col + 2] = b;
	img->data[row + col + 3] = a;
}
void imageloader::setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
	size_t row = y * (img->w * 3);
	size_t col = x * 3;
	img->data[row + col] = r;
	img->data[row + col + 1] = g;
	img->data[row + col + 2] = b;
}

void imageloader::setPixel(IMG img, int x, int y, uint8_t r, uint8_t g) {
	size_t row = y * (img->w * 2);
	size_t col = x * 2;
	img->data[row + col] = r;
	img->data[row + col + 1] = g;
}
void imageloader::setPixel(IMG img, int x, int y, uint8_t r) {
	size_t row = y * (img->w);
	size_t col = x;
	img->data[row + col] = r;
}


void imageloader::setPixel(IMG img, int x, int y, uint32_t color, int bytes) {
	size_t row = y * (img->w * bytes);
	size_t col = x * bytes;
	int shift = (bytes * 8) - 8;
	for (int i = 0; i < bytes; i++) {
		img->data[row + col + i] = (uint8_t)((color>>shift)&0xff);
		shift -= 8;
	}
}

uint64_t imageloader::getPixel(IMG img, int x, int y, int bytes) {
	if (bytes <= 0) {
		return 0;
	}
	size_t row = y * (img->w * bytes);
	size_t col = x * bytes;
	uint32_t out = 0;
	int shift = (bytes * 8) - 8;
	for (int i = 0; i < bytes; i++) {
		out |= img->data[row + col + i] << shift;
		shift -= 8;
	}
	return out;
}
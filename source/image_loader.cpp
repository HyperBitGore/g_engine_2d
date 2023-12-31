#include "g_engine_2d.h"

IMG ImageLoader::generateBlankIMG(int w, int h, int bytes_per_pixel) {
	IMG n_img = new g_img;
	n_img->h = h;
	n_img->w = w;
	n_img->bytes_per_pixel = bytes_per_pixel;
	n_img->data = (unsigned char*)std::malloc((w * bytes_per_pixel) * h); //pixel is four bytes so w*4 is the stride
	std::memset(n_img->data, 0, (w * bytes_per_pixel) * h);
	return n_img;
}

void ImageLoader::createTexture(IMG img, GLenum internalformat, GLenum format) {
	img->pos = cur_tex;
	glCreateTextures_g(GL_TEXTURE_2D, 1, &img->tex);
	//glBindTextureUnit_g(GL_TEXTURE0, img->tex);
	glBindTexture(GL_TEXTURE_2D, img->tex);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D_g(img->tex, 1, internalformat, img->w, img->h);
	glTextureSubImage2D_g(img->tex, 0, 0, 0, img->w, img->h, format, GL_UNSIGNED_BYTE, img->data);
	glGenerateMipmap_g(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	cur_tex++;
}

void ImageLoader::setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	size_t row = y * (img->w * 4);
	size_t col = x * 4;
	img->data[row + col] = r;
	img->data[row + col + 1] = g;
	img->data[row + col + 2] = b;
	img->data[row + col + 3] = a;
}
void ImageLoader::setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
	size_t row = y * (img->w * 3);
	size_t col = x * 3;
	img->data[row + col] = r;
	img->data[row + col + 1] = g;
	img->data[row + col + 2] = b;
}

void ImageLoader::setPixel(IMG img, int x, int y, uint8_t r, uint8_t g) {
	size_t row = y * (img->w * 2);
	size_t col = x * 2;
	img->data[row + col] = r;
	img->data[row + col + 1] = g;
}
void ImageLoader::setPixel(IMG img, int x, int y, uint8_t r) {
	size_t row = y * (img->w);
	size_t col = x;
	img->data[row + col] = r;
}


void ImageLoader::setPixel(IMG img, int x, int y, uint32_t color, int bytes) {
	size_t row = y * (img->w * bytes);
	size_t col = x * bytes;
	int shift = (bytes * 8) - 8;
	for (int i = 0; i < bytes; i++) {
		img->data[row + col + i] = (uint8_t)((color>>shift)&0xff);
		shift -= 8;
	}
}

uint64_t ImageLoader::getPixel(IMG img, int x, int y, int bytes) {
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

IMG ImageLoader::loadPNG(std::string file, unsigned int w, unsigned int h) {
	IMG f = new g_img;
	unsigned error = lodepng_decode32_file((&(f->data)), &w, &h, file.c_str());
	if (error) {
		std::cout << lodepng_error_text(error) << std::endl;
	}
	f->w = w;
	f->h = h;
	f->bytes_per_pixel = 4;
	f->pos = cur_tex;
	
	glGenTextures(1, &f->tex);
	//glCreateTextures_g(GL_TEXTURE_2D, 1, &f->tex);
	//glBindTextureUnit_g(GL_TEXTURE0, f->tex);
	glActiveTexture_g(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, f->tex);
	
	
	glTextureParameteri_g(f->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(f->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(f->tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri_g(f->tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D_g(f->tex, 1, GL_RGBA8, f->w, f->h);
	//glTextureSubImage2D_g(f->tex, 0, 0, 0, f->w, f->h, GL_RGBA, GL_UNSIGNED_BYTE, f->data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, f->data);
	glGenerateMipmap_g(GL_TEXTURE_2D);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	cur_tex++;
	return f;
}

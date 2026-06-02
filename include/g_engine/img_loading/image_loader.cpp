#include "image_loader.hpp"
#include <cstdint>
#include <memory>
#include <stdexcept>

gore::IMG gore::imageloader::createBlank(GLuint w, GLuint h, GLuint bytes_per_pixel){
	IMG img = std::make_unique<g_img>();
	img->h = h;
	img->w = w;
	img->bytes_per_pixel = bytes_per_pixel;
	img->data = new uint8_t[(w * bytes_per_pixel) * h];
	std::memset(img->data, 0, (w * bytes_per_pixel) * h);
	img->size = (w * bytes_per_pixel * h);
	return std::move(img);
}

void gore::imageloader::createTexture(const IMG& img, GLenum internalformat, GLenum format, GLenum type){
	img->format = format;
	img->type = type;
	img->internalformat = internalformat;
	glGenTextures(1, &img->tex);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img->tex);
	glTextureParameteri(img->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(img->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(img->tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(img->tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(img->tex, 1, internalformat, img->w, img->h);
	glTexImage2D(img->tex, 0, 0, 0, img->w, img->h, format, GL_UNSIGNED_BYTE, img->data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

gore::IMG gore::imageloader::copyIMG (const IMG& img) {
	IMG n_img = std::make_unique<g_img>();
	n_img->h = img->h;
	n_img->w = img->w;
	n_img->bytes_per_pixel = img->bytes_per_pixel;
	n_img->data = new uint8_t[(img->w * img->bytes_per_pixel) * img->h];
	std::memset(img->data, 0, (img->w * img->bytes_per_pixel) * img->h);
	n_img->size = (img->w * img->bytes_per_pixel * img->h);
	n_img->format = img->format;
	n_img->type = img->type;
	n_img->internalformat = img->internalformat;
	glGenTextures(1, &img->tex);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img->tex);
	glTextureParameteri(img->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(img->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(img->tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(img->tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(img->tex, 1, n_img->internalformat, img->w, img->h);
	glTexImage2D(img->tex, 0, 0, 0, img->w, img->h, n_img->format, GL_UNSIGNED_BYTE, img->data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	return std::move(n_img);
}


void gore::imageloader::setPixel(const IMG& img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	size_t row = y * (img->w * 4);
	size_t col = x * 4;
	img->data[row + col] = r;
	img->data[row + col + 1] = g;
	img->data[row + col + 2] = b;
	img->data[row + col + 3] = a;
}
void gore::imageloader::setPixel(const IMG& img, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
	size_t row = y * (img->w * 3);
	size_t col = x * 3;
	img->data[row + col] = r;
	img->data[row + col + 1] = g;
	img->data[row + col + 2] = b;
}

void gore::imageloader::setPixel(const IMG& img, int x, int y, uint8_t r, uint8_t g) {
	size_t row = y * (img->w * 2);
	size_t col = x * 2;
	img->data[row + col] = r;
	img->data[row + col + 1] = g;
}
void gore::imageloader::setPixel(const IMG& img, int x, int y, uint8_t r) {
	size_t row = y * (img->w);
	size_t col = x;
	img->data[row + col] = r;
}


void gore::imageloader::setPixelRaw(const IMG& img, int x, int y, uint32_t color, int bytes) {
	if (x >= img->w || y >= img->h) {
		return;
	}
	size_t row = y * (img->w * bytes);
	size_t col = x * bytes;
	int shift = (bytes * 8) - 8;
	for (int i = 0; i < bytes; i++) {
		img->data[row + col + i] = (uint8_t)((color>>shift)&0xff);
		shift -= 8;
	}
}

uint64_t gore::imageloader::getPixel(const IMG& img, int x, int y, int bytes) {
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

gore::IMG gore::imageloader::convertIMGRGBA8(const IMG& img) {
	if (img->format == GL_RGBA8) {
		throw std::runtime_error("Already in RGBA8 format!");
	}
	IMG data = createBlank(img->w, img->h, 4);
	createTexture(data, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
	switch (img->format) {
		case GL_R8:
			for(size_t i = 0, j = 0; i < img->size && j < data->size; i++, j += 4) {
				uint8_t val = img->data[i];
				data->data[j] = val;
				data->data[j + 1] = val;
				data->data[j + 2] = val;
				data->data[j + 3] = 255;
			}
		break;
		case GL_R16:
		{
			uint16_t min_val = UINT16_MAX;
			uint16_t max_val = 0;
			// First pass to find min/max
			for (size_t i = 0; i < img->size; i += 2) {
				uint16_t val = (img->data[i] << 8) | img->data[i + 1];
				if (val < min_val) min_val = val;
				if (val > max_val) max_val = val;
			}
			float range = max_val - min_val;
			if (range == 0) range = 1; // prevent divide by zero
			for(size_t i = 0, j = 0; i < img->size && j < data->size; i+=2, j += 4) {
				uint16_t val = (img->data[i] << 8) | img->data[i + 1];				
				uint8_t norm = ((val - min_val) * 255.0f) / range;
				data->data[j] = norm;
				data->data[j + 1] = norm;
				data->data[j + 2] = norm;
				data->data[j + 3] = 255;
			}
		}
		break;
		case GL_RGB16:
			for(size_t i = 0, j = 0; i < img->size && j < data->size; i += 6, j += 4) {
				uint16_t r = (img->data[i] << 8) | (img->data[i + 1]);
				uint16_t g = (img->data[i + 2] << 8) | (img->data[i + 3]);
				uint16_t b = (img->data[i + 4] << 8) | (img->data[i + 5]);
				data->data[j] = r >> 8;
				data->data[j + 1] = g >> 8;
				data->data[j + 2] = b >> 8;
				data->data[j + 3] = 255;
			}
		break;
		case GL_RGB8:
			for(size_t i = 0, j = 0; i < img->size && j < data->size; i += 3, j += 4) {
				uint8_t r = img->data[i];
				uint8_t g = img->data[i + 1];
				uint8_t b = img->data[i + 2];
				data->data[j] = r;
				data->data[j + 1] = g;
				data->data[j + 2] = b;
				data->data[j + 3] = 255;
			}
		break;
		case GL_RG16:
		{
			uint16_t min_val = UINT16_MAX;
			uint16_t max_val = 0;
			uint16_t alpha_min = UINT16_MAX;
			uint16_t alpha_max = 0;
			// First pass to find min/max
			for (size_t i = 0; i + 3 < img->size; i += 4) {
				uint16_t val = (img->data[i] << 8) | img->data[i + 1];
				uint16_t alpha = (img->data[i + 2] << 8) | (img->data[i + 3]);
				if (val < min_val) min_val = val;
				if (val > max_val) max_val = val;
				if (alpha < alpha_min) alpha_min = alpha;
				if (alpha > alpha_max) alpha_max = alpha;
			}
			float range = max_val - min_val;
			if (range == 0) range = 1; // prevent divide by zero
			float alpha_range = alpha_max - alpha_min;
			if (alpha_range == 0) alpha_range = 1;
			for(size_t i = 0, j = 0; i + 3 < img->size && j < data->size; i+=4, j += 4) {
				uint16_t val = (img->data[i] << 8) | (img->data[i + 1]);
				uint16_t alpha = (img->data[i + 2] << 8) | (img->data[i + 3]);
				float v = ((val - min_val) * 255.0f) / range;
				uint8_t norm = std::roundf(v);
				float av = ((alpha - alpha_min) * 255.0f) / alpha_range;
				uint8_t anorm = std::roundf(av);
				data->data[j] = norm;
				data->data[j + 1] = norm;
				data->data[j + 2] = norm;
				data->data[j + 3] = anorm;
			}
		}
			break;
		case GL_RG8:
			for(size_t i = 0, j = 0; i < img->size && j < data->size; i+=2, j += 4) {
				uint8_t val = img->data[i];
				data->data[j] = val;
				data->data[j + 1] = val;
				data->data[j + 2] = val;
				data->data[j + 3] = img->data[i + 1];
			}
		break;
		case GL_RGBA16:
			for(size_t i = 0, j = 0; i < img->size && j < data->size; i += 8, j += 4) {
				uint16_t r = (img->data[i] << 8) | (img->data[i + 1]);
				uint16_t g = (img->data[i + 2] << 8) | (img->data[i + 3]);
				uint16_t b = (img->data[i + 4] << 8) | (img->data[i + 5]);
				uint16_t a = (img->data[i + 6] << 8) | (img->data[i + 7]);
				data->data[j] = r >> 8;
				data->data[j + 1] = g >> 8;
				data->data[j + 2] = b >> 8;
				data->data[j + 3] = a >> 8;
			}
		break;
		default:
			throw std::runtime_error("Unsupported color type trying to convert img!");
		break;
	}
	updateIMG(data);
	return data;
}

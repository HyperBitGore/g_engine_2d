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
	glBindTextureUnit_g(GL_TEXTURE0, img->tex);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri_g(img->tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D_g(img->tex, 1, internalformat, img->w, img->h);
	glTextureSubImage2D_g(img->tex, 0, 0, 0, img->w, img->h, format, GL_UNSIGNED_BYTE, img->data);
	glGenerateMipmap_g(GL_TEXTURE_2D);

	glBindTextureUnit_g(GL_TEXTURE0, 0);
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



//this doesn't work
void ImageLoader::readBMPPixels32(IMG f, std::stringstream& str, size_t offset, size_t raw_size) {
		//return;
		//char* t = (char*)str.c_str();
		//these are bitmasks
		//std::vector<int> color_table;
		//read color table
		/*size_t off = 54;
		t += 54;
		int* tpo = (int*)t;
		for (off; off < offset; off += 4) {
			color_table.push_back(*tpo);
			tpo++;
		}*/
		//
		//std::string top = str.str();
		//const char* t = top.c_str();
		for (int i = 0; i < 54; i++) {
			str.get();
		}
		std::string top = str.str();
		//t += offset;
		size_t j = 0;
		int c = 0;
		char alpha_val = 0;
		char r_val;
		char g_val;
		char b_val;
		size_t r_spot = 0;
		for (size_t i = offset; i < offset + raw_size; i++, j++) {
			switch (c) {
			case 0:
				r_spot = j;
				alpha_val = top[i];
				break;
			case 1:
				r_val = top[i];
				break;
			case 2:
				g_val = top[i];
				break;
			case 3:
				b_val = top[i];
				//uint32_t p = pack_rgba(r_val, g_val, b_val, 255);
				//p = (p & color_table[alpha_val]);
				f->data[r_spot] = r_val;
				f->data[r_spot + 1] = g_val;
				f->data[r_spot + 2] = b_val;
				f->data[r_spot + 3] = 255;
				c = -1;
				break;
			}
			c++;
		}
	}
	//fix color being off, ie read color table
	void ImageLoader::readBMPPixels24(IMG f, std::string str, size_t offset, size_t raw_size) {
		char* t = (char*)str.c_str();
		char* fo = (char*)f->data;
		t += offset;
		//this is in bgr8  not rgb8 so gotta convert
		size_t j = 0;
		int c = 0;
		size_t rspot = j;
		char r_val;
		char g_val;
		char b_val;
		for (size_t i = 0; i < raw_size; i++, j++) {
			switch (c) {
			case 0:
				rspot = j;
				b_val = t[i];
				break;
			case 1:
				g_val = t[i];
				break;
			case 2:
				r_val = t[i];
				f->data[rspot] = r_val;
				f->data[rspot + 1] = g_val;
				f->data[rspot + 2] = b_val;
				c = -1;
				break;
			}
			c++;
		}
	}
	void ImageLoader::parseBMPData(IMG f, std::stringstream& str, size_t offset, unsigned short bitsperpixel, size_t size) {
		//add reading color table later

		//read pixel data
		switch (bitsperpixel) {
		case 1:
			break;
		case 2:
			break;
		case 4:
			break;
		case 8:
			break;
		case 16:
			break;
		case 24:
			readBMPPixels24(f, str.str(), offset, size);
			glCreateTextures_g(GL_TEXTURE_2D, 1, &f->tex);
			glBindTextureUnit_g(GL_TEXTURE0, f->tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, f->w, f->h, 0, GL_RGB, GL_UNSIGNED_BYTE, f->data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			f->bytes_per_pixel = 3;
			break;
		case 32:
			readBMPPixels32(f, str, offset, size);
			glCreateTextures_g(GL_TEXTURE_2D, 1, &f->tex);
			glBindTextureUnit_g(GL_TEXTURE0, f->tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, f->w, f->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, f->data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			f->bytes_per_pixel = 4;
			break;
		}
	}
	//https://docs.fileformat.com/image/bmp/
	//https://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
	//https://en.wikipedia.org/wiki/BMP_file_format
	//https://www.fileformat.info/format/bmp/egff.htm
	//https://medium.com/sysf/bits-to-bitmaps-a-simple-walkthrough-of-bmp-image-format-765dc6857393
	IMG ImageLoader::loadBMP(std::string file) {
		std::ifstream f;
		f.open(file, std::ios::binary);
		std::stringstream stram;
		stram << f.rdbuf();
		std::string t = stram.str();
		//declaring all the attribute variables
		size_t f_size, offset, width, height, raw_size, num_cols, num_imcols;
		unsigned short bitsperpixel;
		int compression, hor_res, ver_res;
		char* fo = (char*)t.c_str();
		//read first two bytes and make sure they are BM
		if (*fo != 'B' || *(fo + 1) != 'M') {
			return nullptr;
		}
		//now read file size
		fo += 2;
		int* fi = (int*)fo;
		f_size = *fi;
		fo += 8;
		//get offset for image bytes
		fi = (int*)fo;
		offset = *fi;
		//read info header, assuming using info header, add support for older versions later
		fi += 2;
		width = *fi;
		fi++;
		height = *fi;
		short* fs = (short*)fi;
		fs += 3;
		bitsperpixel = *fs;
		fs++;
		fi = (int*)fs;
		compression = *fi;
		fi++;
		raw_size = *fi;
		fi++;
		hor_res = *fi;
		fi++;
		ver_res = *fi;
		fi++;
		num_cols = *fi;
		fi++;
		num_imcols = *fi;
		fi++;
		//read actual file
		IMG out = new g_img;
		out->w = (unsigned int)width;
		out->h = (unsigned int)height;
		out->data = (unsigned char*)std::malloc(raw_size);
		out->pos = cur_tex;
		parseBMPData(out, stram, 54, bitsperpixel, raw_size);
		cur_tex++;
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
		
		//glGenTextures(1, &f->tex);
		glCreateTextures_g(GL_TEXTURE_2D, 1, &f->tex);
		glBindTextureUnit_g(GL_TEXTURE0, f->tex);
		//glActiveTexture_g(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, f->tex);
		
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, f->data);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTextureParameteri_g(f->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri_g(f->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri_g(f->tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri_g(f->tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureStorage2D_g(f->tex, 1, GL_RGBA8, f->w, f->h);
		glTextureSubImage2D_g(f->tex, 0, 0, 0, f->w, f->h, GL_RGBA, GL_UNSIGNED_BYTE, f->data);
		glGenerateMipmap_g(GL_TEXTURE_2D);
		
		//glBindTexture(GL_TEXTURE_2D, 0);
		glBindTextureUnit_g(GL_TEXTURE0, 0);
		cur_tex++;
		return f;
	}
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
		if (cur_y > img->h) {
			img->data = (unsigned char*)std::realloc(img->data, (img->w * 4) * (img->h + 200));
		}

		
		for (int y = 0; y < n_img->h; y++) {
			for (int x = 0; x < n_img->w; x++) {
				uint32_t col = (uint32_t)ImageLoader::getPixel(n_img, x, y, 4);
				ImageLoader::setPixel(img, c_x + x, c_y + y, col, 4);
			}
		}
		

		images.insert(n_img, { c_x, c_y });
	}

	Point ImageAtlas::getImagePos(IMG img) {
		return *images.get(img);
	}

	IMG ImageAtlas::getImg() {
		return img;
	}
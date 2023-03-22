#include "g_engine_2d.h"


void ImageLoader::setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	size_t row = y * (img->w * 4);
	size_t col = x * 4;
	img->data[row + col] = r;
	img->data[row + col + 1] = g;
	img->data[row + col + 2] = b;
	img->data[row + col + 3] = a;
}

void ImageLoader::setPixel(IMG img, int x, int y, uint32_t color) {
	size_t row = y * (img->w * 4);
	size_t col = x * 4;
	img->data[row + col] = unpack_r(color);
	img->data[row + col + 1] = unpack_g(color);
	img->data[row + col + 2] = unpack_b(color);
	img->data[row + col + 3] = unpack_a(color);
}

uint32_t ImageLoader::getPixel(IMG img, int x, int y) {
	size_t row = y * (img->w * 4);
	size_t col = x * 4;
	return pack_rgba(img->data[row + col], img->data[row + col + 1], img->data[row + col + 2], img->data[row + col + 3]);
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
			glGenTextures(1, (GLuint*)&f->tex);
			glBindTexture(GL_TEXTURE_2D, (GLuint)f->tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, f->w, f->h, 0, GL_RGB, GL_UNSIGNED_BYTE, f->data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case 32:
			readBMPPixels32(f, str, offset, size);
			glGenTextures(1, (GLuint*)&f->tex);
			glBindTexture(GL_TEXTURE_2D, (GLuint)f->tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, f->w, f->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, f->data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
		parseBMPData(out, stram, 54, bitsperpixel, raw_size);
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
		glGenTextures(1, (GLuint*)&f->tex);
		glBindTexture(GL_TEXTURE_2D, (GLuint)f->tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, f->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		return f;
	}
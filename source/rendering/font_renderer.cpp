#include "font_renderer.hpp"
#include <algorithm>
#include <cstdint>


bool range(float n, float brange, float trange) {
	return n >= brange && n <= trange;
}


//https://www.youtube.com/watch?v=4bIsntTiKfM
//coding math is the goat
//this can't do collinear lines yet
vec2 getIntersection(Line l1, Line l2) {
	float A1 = (l1.p2.y - l1.p1.y);
	float B1 = (l1.p1.x - l1.p2.x);
	float C1 = (A1 * l1.p1.x + B1 * l1.p1.y);
	float A2 = (l2.p2.y - l2.p1.y);
	float B2 = (l2.p1.x - l2.p2.x);
	float C2 = (A2 * l2.p1.x + B2 * l2.p1.y);
	float denominator = A1 * B2 - A2 * B1;
	
	//checking if line is collinear or parallel
	vec2 delta_l1 = { l1.p2.x - l1.p1.x, l1.p2.y - l1.p1.y };
	vec2 delta_l2 = { l2.p2.x - l2.p1.x, l2.p2.y - l2.p1.y };
	float slope1 = delta_l1.y / delta_l1.x;
	float slope2 = delta_l2.y / delta_l2.x;
	float in1 = l1.p1.y - (slope1 * l1.p1.x);
	float in2 = l2.p1.y - (slope2 * l2.p1.x);

	vec2 inter = { (B2 * C1 - B1 * C2) / denominator, (A1 * C2 - A2 * C1) / denominator };


	if (denominator == 0 && in1 != in2) {
		//line is parallel
		return { -1, -1 };
	}
	else if (in1 == in2) {
		//line is collinear
		return { -2, -2 };
	}
	//checking if intersection lies on the segment
	float rx0 = (inter.x - l1.p1.x) / (l1.p2.x - l1.p1.x);
	float ry0 = (inter.y - l1.p1.y) / (l1.p2.y - l1.p1.y);
	float rx1 = (inter.x - l2.p1.x) / (l2.p2.x - l2.p1.x);
	float ry1 = (inter.y - l2.p1.y) / (l2.p2.y - l2.p1.y);
	if (((rx0 >= 0 && rx0 <= 1) || (ry0 >= 0 && ry0 <= 1)) && ((rx1 >= 0 && rx1 <= 1) || (ry1 >= 0 && ry1 <= 1))) {
		return inter;
	}
	return { -1, -1 };
}

/*float convertToRange(float n, float min, float max, float old_min, float old_max) {
	return ((n - old_min) / (old_max - old_min)) * (max - min) + min;
}*/

gore::RasterGlyph gore::FontRenderer::rasterizeGlyph(gore::Glyph* g, int w, int h, uint32_t color, bool flipx) {
	//have to scale glyph contour points
	std::vector<Line> lines;
	for (size_t i = 0; i < g->contours.size(); i++) {
		Line l = g->contours[i];
		l.p1.x = convertToRange(l.p1.x, 0.0f, (float)w - 1, g->xMin, g->xMax);
		l.p1.y = convertToRange(l.p1.y, 0.0f, (float)h - 1, g->yMin, g->yMax);
		

		l.p2.x = convertToRange(l.p2.x, 0.0f, (float)w - 1, g->xMin, g->xMax);
		l.p2.y = convertToRange(l.p2.y, 0.0f, (float)h - 1, g->yMin, g->yMax);

		lines.push_back(l);
	}
	RasterGlyph r_g;
	std::vector<float> intersections;
	r_g.c = g->c;
	r_g.data = imageloader::createBlank(w, h, 4);
	imageloader::createTexture(r_g.data, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
	//rewrite this myself cause I think the tutorials version is utter dogshit water, 
	struct sortContours {
		bool operator() (Line l1, Line l2) { return l1.p1.y < l2.p1.y; }
	} sortLines;
	std::sort(lines.begin(), lines.end(), sortLines);

	struct sortInters {
		bool operator() (vec2 l1, vec2 l2) { return l1.y < l2.y; }
	} sortVec2;

	//https://stackoverflow.com/questions/3838329/how-can-i-check-if-two-segments-intersect
	//do vertical scanlines
	for (int x = 0; x < w; x++) {
		Line test_line = { {(float)x, 0.0f}, {(float)x, (float)h} };
		std::vector<vec2> inters; //list of intersection points
		std::vector<Line> adds;
		for (size_t i = 0; i < lines.size(); i++) {
			vec2 l = getIntersection(test_line, lines[i]);
			if (l.x >= 0 && l.x <= w) {
				adds.push_back(lines[i]);
				inters.push_back({ (float)x, (float)l.y});
			}
		}
		std::sort(inters.begin(), inters.end(), sortVec2);
		for (size_t i = 1; i < inters.size() && i < adds.size();) {
			float y1 = inters[i - 1].y;
			float y2 = inters[i].y;
			for (int y = (int)y1; y <= y2; y++) {
				imageloader::setPixelRaw(r_g.data, x, y, color, 4);
			}
			if (inters.size() % 2 == 0) {
				i += 2;
			}
			else {
				i++;
			}
		}
		
	}
	//flip the image
	if (flipx) {
		//hacky way to deal with fucked up L's
		for (int y = 0; y < h - 1; y++) {
			for (int x = 0, x1 = w - 1; x <= x1; x++, x1--) {
				uint32_t c1 = (uint32_t)imageloader::getPixel(r_g.data, x, y, 4);
				uint32_t c2 = (uint32_t)imageloader::getPixel(r_g.data, x1, y, 4);
				imageloader::setPixelRaw(r_g.data, x, y, c2, 4);
				imageloader::setPixelRaw(r_g.data, x1, y, c1, 4);
			}
		}
	}
	else {
		for (int y1 = 0, y2 = h - 1; y1 <= y2; y1++, y2--) {
			//flipping the current rows
			unsigned char* c1 = (unsigned char*)std::malloc(w * 4);
			std::memcpy(c1, r_g.data->data + (y1 * (w * 4)), w * 4);
			unsigned char* c2 = r_g.data->data + (y2 * (w * 4));
			std::memcpy(r_g.data->data + (y1 * (w * 4)), c2, w * 4);
			std::memcpy(c2, c1, w * 4);
			std::free(c1);
		}
	}
	return r_g;
}
//flipx vector will decide what glyphs to flip on x axis instead of the normal y axis
void gore::FontRenderer::rasterizeFont(gore::Font* Font, int ptsize, uint32_t color, std::vector<uint16_t> flipx) {
	Font->ptsize = ptsize;
	for (size_t i = 0; i < Font->glyphs.size(); i++) {
		bool flip = false;
		for (auto& j : flipx) {
			if (Font->glyphs[i].c == j) {
				flip = true;
				break;
			}
		}
		Font->r_glyphs.push_back(rasterizeGlyph(&Font->glyphs[i], ptsize, ptsize, color, flip));
		imageloader::updateIMG(Font->r_glyphs[Font->r_glyphs.size() - 1].data);
		//imageloader::createTexture(gore::Font->r_glyphs[gore::Font->r_glyphs.size() - 1].data, GL_RGBA8, GL_RGBA);
		//createTexture(gore::Font->r_glyphs[gore::Font->r_glyphs.size() - 1].data, GL_RGBA8, GL_RGBA);
	}
}

int findFontCharRaster(gore::Font* f, uint16_t c) {
	for (size_t i = 0; i < f->r_glyphs.size(); i++) {
		if (f->r_glyphs[i].c == c) {
			return i;
		}
	}
	return 0;
}
int findFontChar(gore::Font* f, uint16_t c) {
	for (size_t i = 0; i < f->glyphs.size(); i++) {
		if (f->glyphs[i].c == c) {
			return i;
		}
	}
	return 0;
}

void gore::FontRenderer::drawRasterText(gore::Font* Font, imagerenderer* img_r, std::string text, float x, float y, int ptsize) {
	if (Font->r_glyphs.size() <= 0) {
		std::cout << "Trying to draw an empty raster gore::Font " << std::endl;
		return;
	}
	float x1 = x;
	float y1 = y;
	//have to scale images based on ptsize
	float scale = (float)Font->ptsize / ((float)Font->ptsize / (float)ptsize);
	for (size_t i = 0; i < text.size(); i++) {
		if (text[i] >= 33) {
			int index = findFontCharRaster(Font, text[i]);
			img_r->drawImage(Font->r_glyphs[index].data, {x1, y1}, {scale, scale});
			//addImageCall( x1, y1, scale, scale);
			//bindImg(gore::Font->r_glyphs[index].data);
			//renderImgs(true);
		}
		x1 += scale + 2;
	}
	
}
//https://lspwww.epfl.ch/publications/typography/frsa.pdf
//https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization#23880
//2.4.4
//cutout memory inefficient parts of glyph like points
void gore::FontRenderer::drawText(std::string text, gore::Font* Font, float x, float y, int ptsize) {
	float x1 = x;
	float y1 = y;
	for (size_t i = 0; i < text.size(); i++) {
		if (text[i] >= 33) {
			int index = findFontChar(Font, text[i]);
			for (size_t j = 0; j < Font->glyphs[index].contours.size(); j++) {
				Line l = Font->glyphs[index].contours[j];
				//converting line points to ptsize
				l.p1.x = convertToRange(l.p1.x, x1, x1 + ptsize - 1, Font->glyphs[index].xMin, Font->glyphs[index].xMax);
				l.p1.y = convertToRange(l.p1.y, y1, y1 + ptsize - 1, Font->glyphs[index].yMin, Font->glyphs[index].yMax);


				l.p2.x = convertToRange(l.p2.x, x1, x1 + ptsize - 1, Font->glyphs[index].xMin, Font->glyphs[index].xMax);
				l.p2.y = convertToRange(l.p2.y, y1, y1 + ptsize - 1, Font->glyphs[index].yMin, Font->glyphs[index].yMax);
				pr->addLine(l.p1, l.p2);
			}
		}
		//increase the pos by ptsize and a small gap
		x1 += ptsize + 2;
	}
	pr->drawBufferLine();
}
#include "font_renderer.hpp"
#include "font_renderer_shader.hpp"
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

const float DENSITY_CONSTANT = 72.0f; // Points per inch

// issue with Q missing part of glyph when rasterizing is some of y's being negative after scaling
gore::RasterGlyph gore::FontRenderer::rasterizeGlyph(gore::Glyph* g, uint32_t color, int ptsize, uint32_t dpi, gore::Font* Font) {
	//have to scale glyph contour points
	std::vector<Line> lines;
	float scale_factor = (ptsize * dpi) / (DENSITY_CONSTANT * Font->unitsPerEm);
	for (size_t i = 0; i < g->contours.size(); i++) {
		Line l = g->contours[i];
		l.p1.x = l.p1.x * scale_factor;
		l.p1.y = l.p1.y * scale_factor;
		
		l.p2.x = l.p2.x * scale_factor;
		l.p2.y = l.p2.y * scale_factor;

		lines.push_back(l);
	}
	float miny = 0.0f;
	float minx = 0.0f;
	float maxy = 0.0f;
	float maxx = 0.0f;
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines[i].p1.x < minx) {
			minx = lines[i].p1.x;
		}
		if (lines[i].p2.x < minx) {
			minx = lines[i].p2.x;
		}
		if (lines[i].p1.y < miny) {
			miny = lines[i].p1.y;
		}
		if (lines[i].p2.y < miny) {
			miny = lines[i].p2.y;
		}
		if (lines[i].p1.x > maxx) {
			maxx = lines[i].p1.x;
		}
		if (lines[i].p2.x > maxx) {
			maxx = lines[i].p2.x;
		}
		if (lines[i].p1.y > maxy) {
			maxy = lines[i].p1.y;
		}
		if (lines[i].p2.y > maxy) {
			maxy = lines[i].p2.y;
		}
	}
	RasterGlyph r_g;
	std::vector<float> intersections;
	r_g.c = g->c;
	int w = ptsize + (abs((int)minx)); //have to add abs minx to width so we can fit glyphs that go below left side bearing
	int h = ptsize + abs((int)miny); //have to add abs miny to height so we can fit glyphs that go below baseline
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
	if (g->c == 'p') {
		std::cout << "here\n";
	}
	for (int x = (int)minx; x < w; x++) {
		Line test_line = { {(float)x, miny}, {(float)x, (float)h} };
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
				int tempy = y;
				if (miny < 0) {
					if (y < 0) {
						tempy = (abs((int)miny) - abs(y));
					} else {
						tempy = y + (int)abs(miny);
					}
				}
				if (tempy >= h) {
					tempy = h - 1;
				}
				imageloader::setPixelRaw(r_g.data, x, tempy, color, 4);
			}
			if (inters.size() % 2 == 0) {
				i += 2;
			}
			else {
				i++;
			}
		}
		
	}
	for (int y1 = 0, y2 = h - 1; y1 <= y2; y1++, y2--) {
		//flipping the current rows
		unsigned char* c1 = (unsigned char*)std::malloc(w * 4);
		std::memcpy(c1, r_g.data->data + (y1 * (w * 4)), w * 4);
		unsigned char* c2 = r_g.data->data + (y2 * (w * 4));
		std::memcpy(r_g.data->data + (y1 * (w * 4)), c2, w * 4);
		std::memcpy(c2, c1, w * 4);
		std::free(c1);
	}
	return r_g;
}
//flipx vector will decide what glyphs to flip on x axis instead of the normal y axis
void gore::FontRenderer::rasterizeFont(gore::Font* Font, int ptsize, uint32_t dpi, uint32_t color) {
	Font->ptsize = ptsize;
	for (size_t i = 0; i < Font->glyphs.size(); i++) {
		Font->r_glyphs.push_back(rasterizeGlyph(&Font->glyphs[i], color, ptsize, dpi, Font));
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

void gore::FontRenderer::drawRasterText(gore::Font* Font, imagerenderer* img_r, std::string text, float x, float y, int ptsize, uint32_t dpi) {
	if (Font->r_glyphs.size() <= 0) {
		std::cout << "Trying to draw an empty raster gore::Font " << std::endl;
		return;
	}
	float x1 = x;
	float y1 = y;
	//have to scale images based on ptsize
	float scale = (float)Font->ptsize / ((float)Font->ptsize / (float)ptsize);
	float scale_factor = (ptsize * dpi) / (DENSITY_CONSTANT * Font->unitsPerEm);
	for (size_t i = 0; i < text.size(); i++) {
		int index = findFontCharRaster(Font, text[i]);
		float adv_pixels = (float)Font->glyphs[index].advanceWidth * scale_factor;
		if (text[i] >= 33) {
			float tempy = y1;
			if (Font->ptsize < Font->r_glyphs[index].data->h) {
				int dif = Font->r_glyphs[index].data->h - Font->ptsize;
				float diff = (float)dif * scale_factor;
				tempy += diff;
			}
			img_r->drawImage(Font->r_glyphs[index].data, {x1, tempy}, {scale, scale});
			//addImageCall( x1, y1, scale, scale);
			//bindImg(gore::Font->r_glyphs[index].data);
			//renderImgs(true);
		}
		x1 += adv_pixels;
	}
	
}
//https://lspwww.epfl.ch/publications/typography/frsa.pdf
//https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization#23880
//2.4.4
//cutout memory inefficient parts of glyph like points
void gore::FontRenderer::drawText(std::string text, gore::Font* Font, float x, float y, int ptsize, uint32_t dpi) {
	float x1 = x;
	float y1 = y;
	float scale_factor = (ptsize * dpi) / (DENSITY_CONSTANT * Font->unitsPerEm);
	for (size_t i = 0; i < text.size(); i++) {
		int index = findFontChar(Font, text[i]);
		float adv_pixels = (float)Font->glyphs[index].advanceWidth * scale_factor;
		if (text[i] >= 33) {
			//draw the glyph
			for (size_t j = 0; j < Font->glyphs[index].contours.size(); j++) {
				Line l = Font->glyphs[index].contours[j];
				//converting line points to ptsize
				l.p1.x = x1 + (l.p1.x * scale_factor);
				l.p1.y = y1 - (l.p1.y * scale_factor);


				l.p2.x = x1 + (l.p2.x * scale_factor);
				l.p2.y = y1 - (l.p2.y * scale_factor);
				vertexs.push_back(l.p1);
				vertexs.push_back(l.p2);
			}
		}
		x1 += adv_pixels;
	}
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    font_shader.bind();
    glBindVertexArray_g(font_vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData_g(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArrays_g(GL_LINES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray_g(0);
   glDisable(GL_LINE_SMOOTH);
}


gore::FontRenderer::FontRenderer(uint32_t w, uint32_t h) {
	this->width = w;
	this->height = h;
	Matrix ortho = Matrix::calculateOrtho(w, h, w, h);
	vertexs.reserve(1000);
    allocated = 1;
    glGenBuffers_g(1, &vertex_buffer);
	font_shader.compile(vertexShaderSourceFont, fragmentShaderSourceFont);
    font_shader.bind();
    glGenVertexArrays_g(1, &font_vao);
    glBindVertexArray_g(font_vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray_g(0);
    glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    font_shader.setuniform("projection", 1, true, ortho);
}

void gore::FontRenderer::setDimensions (uint32_t width, int32_t height) {
	Matrix ortho = Matrix::calculateOrtho(width, height, this->width, this->height);
	font_shader.bind();
	font_shader.setuniform("projection", 1, true, ortho);
	this->width = width;
	this->height = height;
}
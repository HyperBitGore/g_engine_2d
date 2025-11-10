#include "font_renderer.hpp"
#include "font_renderer_shader.hpp"
#include <algorithm>
#include <cstdint>

//https://www.youtube.com/watch?v=4bIsntTiKfM
//coding math is the goat

const float DENSITY_CONSTANT = 72.0f; // Points per inch
// https://github.com/GreenLightning/gpu-font-rendering#method
IMG gore::FontRenderer::rasterizeGlyph(gore::Glyph* g, uint32_t color, int ptsize, uint32_t dpi, gore::Font* Font) {
	//have to scale glyph contour points
	std::vector<Line> lines;
	int32_t new_ptsize = ptsize;
	if (Font->overlap_simple) {
		new_ptsize *= 2;
	}
	float scale_factor = (new_ptsize * dpi) / (DENSITY_CONSTANT * Font->unitsPerEm);
	for (size_t i = 0; i < g->contours.size(); i++) {
		Line l = g->contours[i];
		l.p1.x = l.p1.x * scale_factor;
		l.p1.y = l.p1.y * scale_factor;
		
		l.p2.x = l.p2.x * scale_factor;
		l.p2.y = l.p2.y * scale_factor;

		lines.push_back(l);
	}

	IMG r_g;
	float miny = g->yMin * scale_factor;
	int w = new_ptsize; //have to add abs minx to width so we can fit glyphs that go below left side bearing
	int h = new_ptsize + abs((int)miny); //have to add abs miny to height so we can fit glyphs that go below baseline
	r_g = imageloader::createBlank(w, h, 4);
	imageloader::createTexture(r_g, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
	//https://stackoverflow.com/questions/3838329/how-can-i-check-if-two-segments-intersect
	if (miny < 0) {
		for (size_t i = 0; i < lines.size(); i++) {
			lines[i].p1.y += std::abs(miny);
			lines[i].p2.y += std::abs(miny);
		}
	}
	// winding
	float minx = (lines[0].p1.x < lines[0].p2.x) ? lines[0].p1.x : lines[0].p2.x;
	for (auto& i : lines) {
		if (i.p1.x < minx) {
			minx = i.p1.x;
		}
		if (i.p2.x < minx) {
			minx = i.p2.x;
		}
	}
	uint32_t rminx = (uint32_t)roundf(minx) - 1;
	for (uint32_t y = 0; y < h; y++) {
		for (uint32_t x = 0; x < w; x++) {
			vec2 origin = { x + 0.0f, y + 0.0f };
			Line ray = {{origin.x, origin.y}, {(float)w, origin.y}};
			int32_t winding = 0;
			for (const Line& line : lines) {
				vec2 p1 = line.p1;
				vec2 p2 = line.p2;

				// Skip horizontal lines
				if (p1.y == p2.y) continue;

				int direction = 1; // upward
				if (p1.y > p2.y) {
					std::swap(p1, p2);
					direction = -1; // downward
				}

				// Ray must be strictly between y1 and y2
				if (origin.y <= p1.y || origin.y > p2.y) continue;

				// Compute intersection X
				float dx = p2.x - p1.x;
				float dy = p2.y - p1.y;
				float intersectX = p1.x + (origin.y - p1.y) * dx / dy;
				// Only count intersections to the right of origin
				if (intersectX > origin.x) {
					winding += direction;
				}
			}
			if (winding != 0) {
				imageloader::setPixelRaw(r_g, x, y, color, 4);
			}
		}
	}
	//flipping the current rows
	for (int y1 = 0, y2 = h - 1; y1 <= y2; y1++, y2--) {
		unsigned char* c1 = (unsigned char*)std::malloc(w * 4);
		std::memcpy(c1, r_g->data + (y1 * (w * 4)), w * 4);
		unsigned char* c2 = r_g->data + (y2 * (w * 4));
		std::memcpy(r_g->data + (y1 * (w * 4)), c2, w * 4);
		std::memcpy(c2, c1, w * 4);
		std::free(c1);
	}
	return r_g;
}
//flipx vector will decide what glyphs to flip on x axis instead of the normal y axis
void gore::FontRenderer::rasterizeFont(gore::Font* Font, int ptsize, uint32_t dpi, uint32_t color) {
	Font->ptsize = ptsize;
	uint32_t w = ptsize * (Font->glyphs.size() / 10);
	uint32_t h = w;
	Font->atlas = ImageAtlas(w*(Font->glyphs.size() / 10), h*(Font->glyphs.size()/10), 4, Font->glyphs.size());
	imageloader::createTexture(Font->atlas.getImg(), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
	for (size_t i = 0; i < Font->glyphs.size(); i++) {
		IMG rg = rasterizeGlyph(&Font->glyphs[i], color, ptsize, dpi, Font);
		std::string name = "";
		name.push_back(Font->glyphs[i].c);
		Font->atlas.addImage(rg, name);
		imageloader::updateIMG(Font->atlas.getImg());
	}
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
	if (Font->atlas.getImg() == nullptr) {
		std::cout << "Trying to draw an empty raster gore::Font " << std::endl;
		return;
	}
	float x1 = x;
	float y1 = y;
	//have to scale images based on ptsize
	float scale = (float)Font->ptsize / ((float)Font->ptsize / (float)ptsize);
	float scale_factor = (ptsize * dpi) / (DENSITY_CONSTANT * Font->unitsPerEm);
	for (size_t i = 0; i < text.size(); i++) {
		int index = findFontChar(Font, text[i]);
		float adv_pixels = (float)Font->glyphs[index].advanceWidth * scale_factor;
		float lsb_pixels = (float)Font->glyphs[index].lsb * scale_factor;
		if (text[i] >= 33) {
			float tempy = y1;
			x1 += lsb_pixels;
			std::string name = "";
			name.push_back(Font->glyphs[index].c);
			vec4 uv = Font->atlas.getImagePos(name, true);
			if (Font->ptsize < uv.w) {
				int dif = uv.w - Font->ptsize;
				float diff = (float)dif * scale_factor;
				tempy += diff;
			}
			img_r->addImageVertex({x1, tempy}, {scale, scale}, uv, 0.0f);
		}
		// this shows something wrong with actual data I am loading, looking at glyph in fontdrop the numbers are off for opensans
		// might be way I am loading the glyph positions????
		if (text[i] == 'o') {
			std::cout << "too low\n";
		}
		x1 += adv_pixels;
	}
	img_r->drawBuffer(Font->atlas.getImg());
}
//https://lspwww.epfl.ch/publications/typography/frsa.pdf
//https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization#23880
//2.4.4
//cutout memory inefficient parts of glyph like points
// have to use LSB LMAO
void gore::FontRenderer::drawText(std::string text, gore::Font* Font, float x, float y, int ptsize, uint32_t dpi) {
	float x1 = x;
	float y1 = y;
	float scale_factor = (ptsize * dpi) / (DENSITY_CONSTANT * Font->unitsPerEm);
	for (size_t i = 0; i < text.size(); i++) {
		int index = findFontChar(Font, text[i]);
		float adv_pixels = (float)Font->glyphs[index].advanceWidth * scale_factor;
		float lsb_pixels = (float)Font->glyphs[index].lsb * scale_factor;
		if (text[i] >= 33) {
			//draw the glyph
			x1 += lsb_pixels;
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
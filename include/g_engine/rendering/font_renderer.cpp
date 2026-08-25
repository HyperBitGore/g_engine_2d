#include "font_renderer.hpp"
#include "font_renderer_shader.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

//https://www.youtube.com/watch?v=4bIsntTiKfM
//coding math is the goat

const float DENSITY_CONSTANT = 72.0f; // Points per inch
// https://github.com/GreenLightning/gpu-font-rendering#method
void gore::fontraster::rasterizeGlyph(gore::glyph* g, uint32_t color, int ptsize, uint32_t dpi, gore::font* Font) {
	//have to scale glyph contour points
	std::vector<line> lines;
	int32_t new_ptsize = ptsize;
	if (Font->overlap_simple) {
		new_ptsize *= 2;
	}
	float scale_factor = (new_ptsize * dpi) / (DENSITY_CONSTANT * Font->unitsPerEm);
	float miny = g->yMin * scale_factor;
	float maxy = g->yMax * scale_factor;
	float minx = g->xMin * scale_factor;
	float maxx = g->xMax * scale_factor;
	float lsbx = g->lsb * scale_factor;
	float rsbx = g->rsb * scale_factor;
	for (size_t i = 0; i < g->contours.size(); i++) {
		line l = g->contours[i];
		l.p1.x = l.p1.x * scale_factor;
		l.p1.y = l.p1.y * scale_factor;
		
		l.p2.x = l.p2.x * scale_factor;
		l.p2.y = l.p2.y * scale_factor;

		lines.push_back(l);
	}

	// need to fix this warping ts out of images
	int w = new_ptsize;
	if (maxx > new_ptsize) {
		w += std::ceil(maxx - new_ptsize) + 1;
	}
	int h = new_ptsize; //have to add abs miny to height so we can fit glyphs that go below baseline
	if (miny < 0) {
		h += abs((int)miny);
		g->y_offset = (h) - ptsize;
	}
	//https://stackoverflow.com/questions/3838329/how-can-i-check-if-two-segments-intersect
	if (miny < 0) {
		for (size_t i = 0; i < lines.size(); i++) {
			lines[i].p1.y += std::abs(miny);
			lines[i].p2.y += std::abs(miny);
		}
	}
	vec2 atlas_pos = Font->atlas.getNextImagePos(w, h);
	// non-zero winding rule btw
	for (uint32_t y = 0; y < h; y++) {
		for (uint32_t x = 0; x < w; x++) {
			vec2 origin = { x + 0.0f, y + 0.0f };
			line ray = {{origin.x, origin.y}, {(float)w, origin.y}};
			int32_t winding = 0;
			for (const line& line : lines) {
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
				uint32_t r_x = atlas_pos.x + x;
				uint32_t r_y = atlas_pos.y + (h - y);

				imageloader::setPixelRaw(Font->atlas.getImg(), r_x, r_y, color, 4);
			}
		}
	}
	std::string name = "";
	name.push_back(g->c);
	Font->atlas.insert(name, w, h, atlas_pos);
}
//flipx vector will decide what glyphs to flip on x axis instead of the normal y axis
void gore::fontraster::rasterizeFont(gore::font* Font, int ptsize, uint32_t dpi, uint32_t color, uint32_t start, uint32_t end) {
	Font->ptsize = ptsize;
	uint32_t w = ptsize * 20;
	uint32_t h = w;
	Font->atlas = imageatlas(w, h, 4, Font->glyphs.size());
	std::cout << "Creating atlas for font " << Font->name << " with dimensions " << w << "x" << h << " and max images " << Font->glyphs.size() << std::endl;
	imageloader::createTexture(Font->atlas.getImg(), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
	for (size_t i = 0; i < Font->glyphs.size(); i++) {
		if (Font->glyphs[i].c >= start && Font->glyphs[i].c <= end) {
			rasterizeGlyph(&Font->glyphs[i], color, ptsize, dpi, Font);
		}
	}
	imageloader::updateIMG(Font->atlas.getImg());
}
int findFontChar(gore::font* f, uint16_t c) {
	for (size_t i = 0; i < f->glyphs.size(); i++) {
		if (f->glyphs[i].c == c) {
			return i;
		}
	}
	return 0;
}

void gore::fontraster::drawRasterText(gore::font* font, image_renderer* img_r, std::u16string text, float x, float y, int ptsize, uint32_t dpi) {
	addRasterText(font, img_r, text, x, y, ptsize, dpi);
	img_r->drawBuffer();
}

void gore::fontraster::drawRasterText(gore::font* font, image_renderer* img_r, std::string text, float x, float y, int ptsize, uint32_t dpi) {
	drawRasterText(font, img_r, gore::fontloader::convertToU16String(text), x, y, ptsize, dpi);
}

void gore::fontraster::addRasterText (gore::font* font, image_renderer* img_r, std::string text, float x, float y, int ptsize, uint32_t dpi) {
	addRasterText(font, img_r, gore::fontloader::convertToU16String(text), x, y, ptsize, dpi);
}
void gore::fontraster::addRasterText(gore::font* font, image_renderer* img_r, std::u16string text, float x, float y, int ptsize, uint32_t dpi) {
	if (font->atlas.getImg() == nullptr) {
		std::cout << "Trying to draw an empty raster gore::Font " << std::endl;
		return;
	}
	float x1 = x;
	float y1 = y;
	//have to scale images based on ptsize
	float scale = (float)font->ptsize / ((float)font->ptsize / (float)ptsize);
	float scale_factor = (ptsize * dpi) / (DENSITY_CONSTANT * font->unitsPerEm);
	for (size_t i = 0; i < text.size(); i++) {
		int index = findFontChar(font, text[i]);
		float adv_pixels = (float)font->glyphs[index].advanceWidth * scale_factor;
		float lsb_pixels = (float)font->glyphs[index].lsb * scale_factor;
		if (text[i] >= 33) {
			float tempy = y1 + (font->glyphs[index].y_offset * scale_factor);
			x1 += lsb_pixels;
			std::string name = "";
			name.push_back(font->glyphs[index].c);
			vec4 uv = font->atlas.getImagePos(name, true);
			if (font->ptsize < uv.w) {
				int dif = uv.w - font->ptsize;
				float diff = (float)dif * scale_factor;
				tempy += diff;
			}
			img_r->addImageVertex(font->atlas.getImg()->tex, {x1, tempy}, {scale, scale}, uv, 0.0f);
		}
		x1 += adv_pixels;
	}
}

//https://lspwww.epfl.ch/publications/typography/frsa.pdf
//https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization#23880
//2.4.4
//cutout memory inefficient parts of glyph like points
// have to use LSB LMAO
void gore::fontrenderer::drawText(std::u16string text, gore::font* font, float x, float y, int ptsize, uint32_t dpi) {
	addText(text, font, x, y, ptsize, dpi);
	drawBuffer();
}

void gore::fontrenderer::drawText(std::string text, gore::font* Font, float x, float y, int ptsize, uint32_t dpi) {
	drawText(gore::fontloader::convertToU16String(text), Font, x, y, ptsize, dpi);
}

void gore::fontrenderer::drawBuffer() {
	if (vertexs.empty()) {
		return;
	}
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    shader.bind();
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArraysExt(GL_LINES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
    glDisable(GL_LINE_SMOOTH);
}

void gore::fontrenderer::addText (std::u16string text, gore::font* font, float x, float y, int ptsize, uint32_t dpi) {
	float x1 = x;
	float y1 = y;
	float scale_factor = (ptsize * dpi) / (DENSITY_CONSTANT * font->unitsPerEm);
	for (size_t i = 0; i < text.size(); i++) {
		int index = findFontChar(font, text[i]);
		float adv_pixels = (float)font->glyphs[index].advanceWidth * scale_factor;
		float lsb_pixels = (float)font->glyphs[index].lsb * scale_factor;
		if (text[i] >= 33) {
			//draw the glyph
			x1 += lsb_pixels;
			for (size_t j = 0; j < font->glyphs[index].contours.size(); j++) {
				line l = font->glyphs[index].contours[j];
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
}
void gore::fontrenderer::addText(std::string text, gore::font* font, float x, float y, int ptsize, uint32_t dpi) {
	addText(gore::fontloader::convertToU16String(text), font, x, y, ptsize, dpi);
}

gore::fontrenderer::fontrenderer(uint32_t w, uint32_t h) : renderer<gore::fontrenderer, gore::vec2> (vertexShaderSourceFont, fragmentShaderSourceFont, w, h) {

}
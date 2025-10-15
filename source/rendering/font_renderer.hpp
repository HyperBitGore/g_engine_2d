#pragma once
#include "primitive_renderer.hpp"

template<class T>
T clamp(T p, T n1, T n2) {
	if (p < n1) {
		p = n1;
	}
	else if (p > n2) {
		p = n2;
	}
	return p;
}






namespace gore {

	struct RasterGlyph {
		uint16_t c;
		IMG data; //img we rasterize to
	};

	struct Glyph {
		uint16_t c; //for unicode
		std::vector<Line> contours;
		int16_t yMax;
		int16_t yMin;
		int16_t xMax;
		int16_t xMin;
		uint16_t advanceWidth;
		int16_t lsb; //left side bearing
	};

	//speed this up with a hashmap when done
	struct Font {
		std::string name;
		std::vector<Glyph> glyphs;
		std::vector<RasterGlyph> r_glyphs; //can ignore this if don't plan on using my rasterization
		int ptsize;
		uint16_t unitsPerEm;
	};










	// https://axleos.com/writing-a-truetype-font-renderer/
	// Render ur favorite fonts
	class FontRenderer {
		private:
		std::vector<vec2> vertexs;
		GLuint vertex_buffer;
		GLuint allocated;
		Shader font_shader;
		GLuint font_vao;
		uint32_t width, height;
		public:
		FontRenderer(uint32_t w, uint32_t h);
		//font loading
		Font loadFont(std::string file, uint16_t start, uint16_t end);
		//font drawing
		void drawText(std::string text, Font* font, float x, float y, int ptsize, uint32_t dpi);
		RasterGlyph rasterizeGlyph(Glyph* g, uint32_t color, int ptsize, uint32_t dpi, Font* Font);
		void rasterizeFont(Font* font, int ptsize, uint32_t dpi, uint32_t color);
		void drawRasterText(Font* font, imagerenderer* img_r, std::string text, float x, float y, int ptsize, uint32_t dpi);
		// for width and height
		void setDimensions (uint32_t width, int32_t height);
		// set color of rendered text
		void setColor(vec4 color) {
			font_shader.bind();
			font_shader.setuniform("textColor", color);
		}
	};

}
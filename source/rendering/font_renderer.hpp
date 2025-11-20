#pragma once
#include "primitive_renderer.hpp"
#include "../file_loading/font_loader.hpp"
#include <string>

namespace gore {
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
		//font drawing
		void drawText(std::string text, gore::Font* font, float x, float y, int ptsize, uint32_t dpi);
		void drawText(std::u16string text, gore::Font* font, float x, float y, int ptsize, uint32_t dpi);
		IMG rasterizeGlyph(gore::Glyph* g, uint32_t color, int ptsize, uint32_t dpi, gore::Font* Font);
		void rasterizeFont(gore::Font* font, int ptsize, uint32_t dpi, uint32_t color, uint32_t start, uint32_t end);
		void drawRasterText(gore::Font* font, imagerenderer* img_r, std::string text, float x, float y, int ptsize, uint32_t dpi);
		void drawRasterText(gore::Font* font, imagerenderer* img_r, std::u16string text, float x, float y, int ptsize, uint32_t dpi);
		// for width and height
		void setDimensions (uint32_t width, int32_t height);
		// set color of rendered text
		void setColor(vec4 color) {
			font_shader.bind();
			font_shader.setuniform("textColor", color);
		}
		// if your char codes go above 127, we read it like UTF-8
		static std::u16string convertToU16String (std::string str);
	};
}
#pragma once
#include "primitive_renderer.hpp"
#include "../file_loading/font_loader.hpp"
#include <string>

namespace gore {
	// https://axleos.com/writing-a-truetype-font-renderer/
	// Render ur favorite fonts
	class fontrenderer {
		private:
		std::vector<vec2> vertexs;
		GLuint vertex_buffer;
		GLuint allocated;
		shader font_shader;
		GLuint font_vao;
		uint32_t width, height;
		public:
		fontrenderer(uint32_t w, uint32_t h);
		// copy
		fontrenderer (const fontrenderer& f);
		//font drawing
		void drawText(std::string text, gore::font* font, float x, float y, int ptsize, uint32_t dpi);
		void drawText(std::u16string text, gore::font* font, float x, float y, int ptsize, uint32_t dpi);
		// for width and height
		void setDimensions (uint32_t width, int32_t height);
		// set color of rendered text
		void setColor(vec4 color) {
			font_shader.bind();
			font_shader.setuniform("set_color", color);
		}
	};

	class fontraster {
		public:
		static void rasterizeGlyph(gore::glyph* g, uint32_t color, int ptsize, uint32_t dpi, gore::font* Font);
		static void rasterizeFont(gore::font* font, int ptsize, uint32_t dpi, uint32_t color, uint32_t start, uint32_t end);
		static void drawRasterText(gore::font* font, imagerenderer* img_r, std::string text, float x, float y, int ptsize, uint32_t dpi);
		static void drawRasterText(gore::font* font, imagerenderer* img_r, std::u16string text, float x, float y, int ptsize, uint32_t dpi);

	};
}
#pragma once
#include "renderer.hpp"
#include "image_renderer.hpp"
#include "../file_loading/font_loader.hpp"
#include <string>

namespace gore {
	// https://axleos.com/writing-a-truetype-font-renderer/
	// Render ur favorite fonts
	class fontrenderer : public renderer<fontrenderer, gore::vec2> {
		protected:
		friend class renderer<fontrenderer, gore::vec2>;
		void shader_setup () override {
			gl_function_tagger tags({
				"glBindVertexArray",
				"glBindBuffer",
				"glEnableVertexAttribArray",
				"glVertexAttribPointer",
				"glBufferData",
				"glBufferSubData",
				"glDrawArrays"
			});
			try {
				tags.hardwareSupports();
			} catch (render_function_not_supported& e) {

			}
			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
			updateDimensions(this->width, this->height);
			updateView(0.0f, 0.0f, 1.0f);
			this->draw_arrays_mode = GL_LINES;
		}
		fontrenderer(uint32_t w, uint32_t h);
		public:
		//font drawing
		void drawText(std::string text, gore::font* font, float x, float y, int ptsize, uint32_t dpi);
		void drawText(std::u16string text, gore::font* font, float x, float y, int ptsize, uint32_t dpi);
		// set color of rendered text
		void setColor(vec4 color) {
			shader.bind();
			shader.setuniform("set_color", color);
		}
	};

	class fontraster {
		public:
		static void rasterizeGlyph(gore::glyph* g, uint32_t color, int ptsize, uint32_t dpi, gore::font* Font);
		static void rasterizeFont(gore::font* font, int ptsize, uint32_t dpi, uint32_t color, uint32_t start, uint32_t end);
		static void drawRasterText(gore::font* font, image_renderer* img_r, std::string text, float x, float y, int ptsize, uint32_t dpi);
		static void drawRasterText(gore::font* font, image_renderer* img_r, std::u16string text, float x, float y, int ptsize, uint32_t dpi);

	};
}
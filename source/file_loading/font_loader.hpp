#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include "../img_loading/image_loader.hpp"
#include "../rendering/primitive_renderer.hpp"

namespace gore {
	struct Glyph {
		uint16_t c; //for unicode
		std::vector<Line> contours;
		int16_t yMax;
		int16_t yMin;
		int16_t xMax;
		int16_t xMin;
		int16_t xPos = 0;
		int16_t yPos = 0;
		uint16_t advanceWidth;
		int16_t lsb; //left side bearing
        int16_t rsb; // derived, we calculate this
		int y_offset = -1;
	};
    
    class Font {
		private:
			struct gpos_lookup {
				
			};
		public:
		std::string name;
		std::vector<Glyph> glyphs;
		std::vector<IMG> rastered;
		int32_t ptsize;
		uint16_t unitsPerEm;
		bool overlap_simple;
        ImageAtlas atlas;
		Font () {
			this->name = "";
		}
		Font (std::string name, int32_t ptsize, uint16_t unitsPerEm, bool overlap_simple) {
			this->name = name;
			this->ptsize = ptsize;
			this->unitsPerEm = unitsPerEm;
			this->overlap_simple = overlap_simple;

		}
		// copy
		Font (const Font& font) {
			this->name = font.name;
			this->glyphs = font.glyphs;
			this->ptsize = font.ptsize;
			this->unitsPerEm = font.unitsPerEm;
			this->overlap_simple = font.overlap_simple;
			this->rastered = font.rastered;
		}
		// move
		Font (const Font&& font) {
			this->name = font.name;
			this->glyphs = std::move(font.glyphs);
			this->ptsize = font.ptsize;
			this->unitsPerEm = font.unitsPerEm;
			this->overlap_simple = font.overlap_simple;
			this->rastered = font.rastered;
		}
		// operator=
		Font& operator=(const Font& font) {
			this->name = font.name;
			this->glyphs = font.glyphs;
			this->ptsize = font.ptsize;
			this->unitsPerEm = font.unitsPerEm;
			this->overlap_simple = font.overlap_simple;
			this->rastered = font.rastered;
			return *this;
		}
	};
    class FontLoader {
        public:
	    static Font loadFont(std::string file, uint16_t start, uint16_t end);
    };
}
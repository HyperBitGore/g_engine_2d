#include "font_renderer.hpp"
#include <cstdint>
#include <fstream>
#include <sstream>

#define SwapTwoBytes(data) \
( (((data) >> 8) & 0x00FF) | (((data) << 8) & 0xFF00) ) 

#define SwapFourBytes(data)   \
( (((data) >> 24) & 0x000000FF) | (((data) >>  8) & 0x0000FF00) | \
  (((data) <<  8) & 0x00FF0000) | (((data) << 24) & 0xFF000000) ) 

#define SwapEightBytes(data)   \
( (((data) >> 56) & 0x00000000000000FF) | (((data) >> 40) & 0x000000000000FF00) | \
  (((data) >> 24) & 0x0000000000FF0000) | (((data) >>  8) & 0x00000000FF000000) | \
  (((data) <<  8) & 0x000000FF00000000) | (((data) << 24) & 0x0000FF0000000000) | \
  (((data) << 40) & 0x00FF000000000000) | (((data) << 56) & 0xFF00000000000000) ) 

// compound glyf flags
#define ARG_1_AND_2_ARE_WORDS 0x0001
#define MORE_COMPONENTS 0x0020
#define WE_HAVE_A_SCALE 0x0008
#define WE_HAVE_AN_X_AND_Y_SCALE 0x0040
#define WE_HAVE_A_TWO_BY_TWO 0x0080
#define WE_HAVE_INSTRUCTIONS 0x0100
// simple glyf flags
#define ON_CURVE_POINT 0x01
#define X_SHORT_VECTOR 0x02
#define Y_SHORT_VECTOR 0x04
#define REPEAT_FLAG 0x08
#define X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR 0x10
#define Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR 0x20
#define OVERLAP_SIMPLE 0x40

int getnthBit(short number, int n) {
	return (number >> n) & 1;
}
int getnthBit(uint8_t number, int n) {
	return (number >> n) & 1;
}


uint8_t swap1Byte(uint8_t n) {
	uint8_t n2 = 0;
	for (int i = 0, j = 7; i < 8; i++, j--) {
		int s = getnthBit(n, j);
		if (s == 1) {
			n2 |= 1 << i;
		}
	}
	return n2;
	//return (((n >> 7) & 1) | ((n >> 6) & 1) | ((n >> 5) & 1) | ((n >> 4) & 1) | ((n >> 3) & 1) | ((n >> 2) & 1) | ((n >> 1) & 1) | ((n >> 0) & 1));
}


//ttf file structs

struct off_subtable {
	uint32_t scaler_type; //tag to indicate scaler to be used to razterize gore::Font
	uint16_t numTables; //number of tables
	uint16_t searchRange; //(maximum power of 2 <= numTables)*16
	uint16_t entrySelector; //log2(maximum power of 2 <= numTables)
	uint16_t rangeShift; //numTables*16-searchRange
};

struct table_dir {
	std::string t; //human readable tag
	uint32_t tag; //4-byte identifier
	uint32_t checksum; //checksum for the table
	uint32_t offset; //offset from bneging of 'sfnt' (begining of file)
	uint32_t length; //length of table in bytes
};


struct Font_dir {
	off_subtable off_sub;
	std::vector<table_dir> table;
};

struct glyph_index {
	int index;
	uint16_t c;
};

struct cmap_table {
	uint16_t platformID;
	uint16_t platformSpecificID;
	uint32_t offset;
	std::vector<glyph_index> indexs;
};



struct cmap {
	uint16_t version;
	uint16_t numTables;
	std::vector<cmap_table> tables;
};


//have to convert these to little endian( I don't know how these macros work but they do so fuck it)
void read_offset_subtable(char* c, off_subtable* table){
	uint32_t* t = (uint32_t*)c;
	table->scaler_type = SwapFourBytes(*t);
	t++;
	uint16_t* te = (uint16_t*)t;
	table->numTables = SwapTwoBytes(*te);
	te++;
	table->searchRange = SwapTwoBytes(*te);
	te++;
	table->entrySelector = SwapTwoBytes(*te);
	te++;
	table->rangeShift = SwapTwoBytes(*te);
	te++;
	//c = (char**)te;
}

void read_table_directory(char* c, std::vector<table_dir>& table, int tbl_size) {
	uint32_t* t = (uint32_t*)c;
	for (int i = 0; i < tbl_size; i++) {
		table_dir dir;
		dir.t = "";
		for (int j = 0; j < 4; j++) {
			dir.t.push_back(*c);
			c++;
		}
		dir.tag = SwapFourBytes(*t);
		t++;
		dir.checksum = SwapFourBytes(*t);
		t++;
		dir.offset = SwapFourBytes(*t);
		t++;
		dir.length = SwapFourBytes(*t);
		t++;
		c += 12;
		table.push_back(dir);
	}
}

struct format4 {
	uint16_t format;
	uint16_t length;
	uint16_t language;
	uint16_t segCountX2;
	uint16_t searchRange;
	uint16_t entrySelector;
	uint16_t rangeShift;
	std::vector<uint16_t> endcode;
	uint16_t reservedPad; //left for padding
	std::vector<uint16_t> startCode;
	std::vector<uint16_t> idDelta;
	std::vector<uint16_t> idRangeOffset;
	std::vector<uint16_t> glyphIndexArray;
};
int get_glyph_index_format4(uint16_t c, format4* f, uint16_t* idRangeStart) {
	int index = -1;
	for (int i = 0; i < f->segCountX2 / 2; i++) {
		if (f->endcode[i] > c) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		return 0;
	}
	if (f->startCode[index] < c) {
		uint16_t* ptr = nullptr;
		if (f->idRangeOffset[index] != 0) {
			ptr = idRangeStart + index + f->idRangeOffset[index] / 2;
			ptr += c - f->startCode[index];
			if (SwapTwoBytes(*ptr) == 0) { return 0; }
			return SwapTwoBytes(*ptr) + f->idDelta[index];
		}
		else {
			return c + f->idDelta[index];
		}
	}

	return 0;
}

//c should be at the start of the cmap table so the table.offset works
void readFormat4(char* c, cmap_table* table, uint16_t start, uint16_t end) {
	//now we get to reading, probably make cmap_table have array of glyph indexs so easier to store
	char* m = c + table->offset;
	uint16_t* t = (uint16_t*)m;
	format4 form;
	form.format = SwapTwoBytes(*t);
	t++;
	form.length = SwapTwoBytes(*t);
	t++;
	form.language = SwapTwoBytes(*t);
	t++;
	form.segCountX2 = SwapTwoBytes(*t);
	t++;
	form.searchRange = SwapTwoBytes(*t);
	t++;
	form.entrySelector = SwapTwoBytes(*t);
	t++;
	form.rangeShift = SwapTwoBytes(*t);
	t++;
	//now we read through the rest of the data of the format
	//get the actual correct values
	for (int i = 0; i < form.segCountX2 / 2; i++) {
		form.endcode.push_back(SwapTwoBytes(*(t + i)));

	}
	t += form.segCountX2 / 2 + 1; //add one because there is apparentaly a padding two bytes between endcode list and rest
	for (int i = 0; i < form.segCountX2 / 2; i++) {
		form.startCode.push_back(SwapTwoBytes(*(t + i)));

	}
	t += form.segCountX2 / 2;
	for (int i = 0; i < form.segCountX2 / 2; i++) {
		form.idDelta.push_back(SwapTwoBytes(*(t + i)));

	}
	t += form.segCountX2 / 2;
	uint16_t* idRangeStart = t;
	for (int i = 0; i < form.segCountX2 / 2; i++) {
		form.idRangeOffset.push_back(SwapTwoBytes(*(t + i)));

	}
	t += form.segCountX2 / 2;
	//now we read the glyphidarray
	int remaining = form.length - (((char*)t) - m);

	for (int i = 0; i < remaining / 2; i++) {
		form.glyphIndexArray.push_back(SwapTwoBytes(*(t + i)));
	}
	//now we read all of the character codes, change back to 32
	uint16_t start1 = start;
	for (; start1 <= end; start1++) {
		table->indexs.push_back({ get_glyph_index_format4(start1, &form, idRangeStart), start1 });
	}
	//std::cout << table->indexs[0].c << " : " << table->indexs[0].index << "\n";
}
//untested
void readFormat0(char* c, cmap_table* table, uint16_t start, uint16_t end) {
	char* m = c + table->offset;
	uint16_t* f = (uint16_t*)m;
	f++;
	uint16_t length = SwapTwoBytes(*f);
	f++;
	uint16_t language = SwapTwoBytes(*f);
	f++;
	uint8_t* g = (uint8_t*)f;
	std::vector<uint8_t> id_array;
	for (int i = 0; i < length; i++) {
		id_array.push_back(*(g + i));
	}
	uint16_t start1 = start;
	for (; start1 <= end; start1++) {
		if (start1 < id_array.size()) {
			table->indexs.push_back({ id_array[start1], start1 });
		}
	}
}
//untested and unfinished
void readFormat2(char* c, cmap_table* table, uint16_t start, uint16_t end) {
	char* m = c + table->offset;
	uint16_t* f = (uint16_t*)m;
	f++;
	uint16_t length = SwapTwoBytes(*f);
	f+=2; //skipping language 

}
//untested
void readFormat6(char* c, cmap_table* table, uint16_t start, uint16_t end) {
	char* m = c + table->offset;
	uint16_t* f = (uint16_t*)m;
	uint16_t format = SwapTwoBytes(*f);
	f++;
	uint16_t length = SwapTwoBytes(*f);
	f += 2;//skipping language
	uint16_t firstcode = SwapTwoBytes(*f);
	f++;
	uint16_t entrycount = SwapTwoBytes(*f);
	f++;
	//now we read the glyphidarray
	std::vector<uint16_t> glyphidarray;
	for (uint16_t i = 0; i < entrycount; i++) {
		glyphidarray.push_back(*(f + i));
	}

	//outputting to the table
	uint16_t start1 = start;
	for (; start1 < end; start1++) {
		int offset = start1 - firstcode;
		if (offset > 0 && offset < entrycount) {
			table->indexs.push_back({ glyphidarray[offset], start1 });
		}
		else {
			table->indexs.push_back({ glyphidarray[0], start1 });
		}
	}
}
void readFormat8(char* c, cmap_table* table, uint16_t start, uint16_t end) {
	char* m = c + table->offset;
	uint16_t* f16 = (uint16_t*)m;
	f16 += 2;//skipping format and reserved
	uint32_t* f32 = (uint32_t*)f16;
	uint32_t length = *f32;
	f32 += 2;//skipping language
	//now we read the packed array of bits

}

//untested and im unsure if this is proper way to read this format
void readFormat10(char* c, cmap_table* table, uint16_t start, uint16_t end) {
	char* m = c + table->offset;
	uint16_t* f16 = (uint16_t*)m;
	f16 += 2; //skipping first two
	uint32_t* f32 = (uint32_t*)f16;
	uint32_t length = *f32;
	f32+=2;//skipping language
	uint32_t starcharcode = *f32;
	f32++;
	uint32_t numChars = *f32;
	f32++;
	f16 = (uint16_t*)f32;
	std::vector<uint16_t> glyphindices;
	for (size_t i = 0; i < numChars; i++) {
		glyphindices.push_back(*(f16 + i));
	}
	uint16_t start1 = start;
	for (; start1 < end; start1++) {
		int offset = start1 - starcharcode;
		table->indexs.push_back({ glyphindices[offset], start1 });
	}
}
//untested and not done
void readFormat12(char* c, cmap_table* table, uint16_t start, uint16_t end) {
	char* m = c + table->offset;
	uint16_t* f = (uint16_t*)m;
	f += 2;
	//skipping the format and reserved
	uint32_t* t = (uint32_t*)f;
	uint32_t length = *t;
	t += 2; //skipping language
	uint32_t numGroups = *t; //number of groupings that follow
	t++;
	std::vector<uint32_t> char_codes;
	std::vector<uint32_t> indexs;
	size_t seq = 0;
	for (size_t i = 0; i < numGroups; i++) {
		seq = 0;
		uint32_t startcode = *t;
		t++;
		uint32_t endcode = *t;
		t++;
		uint32_t startglyphid = *t;
		t++;
		for (size_t j = startcode; j <= endcode; j++, seq++) {
			char_codes.push_back(*t);
			indexs.push_back((GLuint)startglyphid + (GLuint)seq);
			t++;
		}
	}
	uint16_t start1 = start;
	for (; start1 < end; start1++) {
		for (size_t i = 0; i < char_codes.size(); i++) {
			if (char_codes[i] == start1) {
				table->indexs.push_back({ (uint16_t)indexs[i], start1});
				break;
			}
		}
		
	}
}

//https://developer.apple.com/gore::Fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
//https://learn.microsoft.com/en-us/typography/opentype/spec/cmap
cmap readCmap(char* c, int offset, int length, uint16_t start, uint16_t end) {
	cmap map;
	char* m = c + offset;
	uint16_t* t = (uint16_t*)m;
	map.version = SwapTwoBytes(*t);
	t++; //skipping version cause I don't care
	map.numTables = SwapTwoBytes(*t);
	t++;
	//now we read the subtables
	for (int i = 0; i < (int)map.numTables; i++) {
		cmap_table table;
		table.platformID = SwapTwoBytes(*t);
		t++;
		table.platformSpecificID = SwapTwoBytes(*t);
		t++;
		uint32_t* te = (uint32_t*)t;
		table.offset = SwapFourBytes(*te);
		te++;
		t = (uint16_t*)te;
		map.tables.push_back(table); //offset is from start of cmap
		int format = (m + map.tables[i].offset)[0] << 8 | (m + map.tables[i].offset)[1];
		switch (format) {
		case 4:
			//most common
			readFormat4(m, &map.tables[i], start, end);
			break;
		case 0:
			readFormat0(m, &map.tables[i], start, end);
			break;
		case 2:
			std::cout << "Unsupported cmap format; Format 2;" << std::endl;
			//readFormat2(m, &map.tables[i], start, end);
			break;
		case 6:
			readFormat6(m, &map.tables[i], start, end);
			break;
		case 8:
			std::cout << "Unsupported cmap format; Format 8;" << std::endl;
			//readFormat8(m, &map.tables[i], start, end);
			break;
		case 10:
			readFormat10(m, &map.tables[i], start, end);
			break;
		case 12:
			//most common
			readFormat12(m, &map.tables[i], start, end);
			break;
		case 13:
			//not doing this cause it is not needed
			std::cout << "Unsupported cmap format; Format 13;" << std::endl;
			break;
		case 14:
			//not doing this cause it is not needed
			std::cout << "Unsupported cmap format; Format 14;" << std::endl;
			break;
		}
	}
	


	return map;
}

//stores character code and offset
struct loca {
	uint16_t c;
	uint32_t offset;
};



//read the table when it is the 16 bit version
uint32_t readLoca16(char* s, uint16_t index) {
	uint16_t* d = ((uint16_t*)s + index);
	return SwapTwoBytes(*d) * 2;
}
//reading the table when it is the 32 bit version
uint32_t readLoca32(char* s, uint16_t index) {
	uint32_t* d = (uint32_t*)s + index;
	return SwapFourBytes(*d);
}

std::vector<loca> readLoca(char* c, int offset, int length, uint16_t format, cmap* map) {
	char* m = c + offset;
	std::vector<loca> locas;
	int index = 0;
	for (size_t i = 0; i < map->tables.size(); i++) {
		if (map->tables[i].platformSpecificID == 3) {
			index = (int)i;
			break;
		}
	}

	if (format == 0) {
		for (size_t i = 0; i < map->tables[index].indexs.size(); i++) {
			loca l;
			l.c = map->tables[index].indexs[i].c;
			l.offset = readLoca16(m, map->tables[index].indexs[i].index);
			locas.push_back(l);
		}
	}
	else {
		for (size_t i = 0; i < map->tables[index].indexs.size(); i++) {
			loca l;
			l.c = (map->tables[index].indexs[i].c);
			l.offset = readLoca32(m, map->tables[index].indexs[i].index);
			locas.push_back(l);
		}
	}
	return locas;
}

//most of data in here doesn't matter for my uses
struct TTFHeader {
	uint32_t version; //supposed to be fixed but idrc
	uint32_t FontRevision; //set by manufacturer

	uint32_t checkSumAdjustment; //checksum for file; Have to sum entire file as an uint32_t and then do 0xB1B0AFBA - sum; 
	uint32_t magicNumber; //idek what this is for supposed to be 0x5F0F3CF5

	uint16_t flags; //every bit gives u flag; These flags can be complex as shit
	uint16_t uintsPerEm; //how may FUnits are in 1 em

	time_t  created; //self explanatory
	time_t modified; //also self explanatory

	short xMin; //supposed to be an FWord but that is just 16 bit signed integer so a short; Also in FUnits
	short yMin;
	short xMax;
	short yMax;

	uint16_t macStyle; //each bit has parameters
	uint16_t lowestRecPPEM; //smallest readable size in pixels
	short FontDirectionHint; //give you hint for directions of glyphs
	short indexToLocFormat; //tells you format of loca table
	short glyphDataFormat; //0 is current format; All i know
};



TTFHeader readHead(char* c, int offset, int length) {
	TTFHeader head;
	char* m = c + offset;
	uint32_t* u = (uint32_t*)m;
	head.version = SwapFourBytes(*u); //can't swap the bytes on a double for some reason
	u++;
	head.FontRevision = SwapFourBytes(*u); //values will be wrong for the top two since supposed to be fixed
	u++;
	head.checkSumAdjustment = SwapFourBytes(*u);
	u++;
	head.magicNumber = SwapFourBytes(*u);
	u++;
	uint16_t* t = (uint16_t*)u;
	head.flags = *t; //don't swap this since the bits need to be the same
	t++;
	head.uintsPerEm = SwapTwoBytes(*t);
	t++;
	time_t* p = (time_t*)t;
	head.created = SwapEightBytes(*p);
	p++;
	head.modified = SwapEightBytes(*p);
	p++;
	short* s = (short*)p;
	head.xMin = SwapTwoBytes(*s);
	s++;
	head.yMin = SwapTwoBytes(*s);
	s++;
	head.xMax = SwapTwoBytes(*s);
	s++;
	head.yMax = SwapTwoBytes(*s);
	s++;
	t = (uint16_t*)s;
	head.macStyle = *t; //don't swap cause need same bits for flags
	t++;
	head.lowestRecPPEM = SwapTwoBytes(*t);
	t++;
	s = (short*)t;
	head.FontDirectionHint = SwapTwoBytes(*s); 
	s++;
	head.indexToLocFormat = SwapTwoBytes(*s);
	s++;
	head.glyphDataFormat = SwapTwoBytes(*s);
	s++;
	return head;
}

struct hhea_table {
	uint16_t majorVersion; // should be 1
	uint16_t minorVersion; // should be 0
	int16_t ascender; //typical height above baseline
	int16_t descender; //typical depth below baseline
	int16_t lineGap; //typical gap between lines
	uint16_t advanceWidthMax; //maximum advance width value in 'hmtx' table
	int16_t minLeftSideBearing; //minimum left sidebearing value in 'hmtx' table
	int16_t minRightSideBearing; //minimum right sidebearing value in 'hmtx' table
	int16_t xMaxExtent; //max(lsb + (xMax-xMin))
	int16_t caretSlopeRise; //used to calculate the slope of the cursor (rise/run); 1 for vertical
	int16_t caretSlopeRun; //0 for vertical
	int16_t caretOffset; //set to 0 for non-slanted fonts
	//int16_t reserved[4]; //set to 0
	int16_t metricDataFormat; //0 for current format
	uint16_t numberOfHMetrics; //number of hMetric entries in 'hmtx' table
};

hhea_table readHheaTable(char* c, int offset, int length) {
	hhea_table h;
	char* m = c + offset;
	int16_t* t = (int16_t*)m;
	h.majorVersion = SwapTwoBytes(*t);
	t++;
	h.minorVersion = SwapTwoBytes(*t);
	t++;
	h.ascender = SwapTwoBytes(*t);
	t++;
	h.descender = SwapTwoBytes(*t);
	t++;
	h.lineGap = SwapTwoBytes(*t);
	t++;
	h.advanceWidthMax = SwapTwoBytes(*t);
	t++;
	h.minLeftSideBearing = SwapTwoBytes(*t);
	t++;
	h.minRightSideBearing = SwapTwoBytes(*t);
	t++;
	h.xMaxExtent = SwapTwoBytes(*t);
	t++;
	h.caretSlopeRise = SwapTwoBytes(*t);
	t++;
	h.caretSlopeRun = SwapTwoBytes(*t);
	t++;
	h.caretOffset = SwapTwoBytes(*t);
	t++;
	t += 4; //skipping reserved
	h.metricDataFormat = SwapTwoBytes(*t);
	t++;
	h.numberOfHMetrics = SwapTwoBytes(*t);
	t++;

	return h;
}

struct long_hor_metric {
	uint16_t advanceWidth;
	int16_t lsb; //left side bearing
};

struct hmtx_table {
	std::vector<long_hor_metric> hMetrics;
	std::vector<int16_t> leftSideBearings; //for glyphs that have same width as previous glyph
};

hmtx_table readHmtxTable(char* c, int offset, int length, uint16_t numHMetrics, size_t numGlyphs) {
	hmtx_table h;
	char* m = c + offset;
	uint16_t* t = (uint16_t*)m;
	for (size_t i = 0; i < numHMetrics; i++) {
		long_hor_metric hm;
		hm.advanceWidth = SwapTwoBytes(*t);
		t++;
		hm.lsb = SwapTwoBytes(*t);
		t++;
		h.hMetrics.push_back(hm);
	}
	//now we read the left side bearings
	int16_t* s = (int16_t*)t;
	for (size_t i = numHMetrics; i < numGlyphs; i++) {
		h.leftSideBearings.push_back(SwapTwoBytes(*s));
		s++;
	}
	return h;
}

struct glyf {
	uint16_t c;
	short numberOfContours;

	//supposed to be FWords but fuck em
	short xMin;
	short yMin;
	short xMax;
	short yMax;
};



struct simp_glyf : glyf {
	uint16_t instructionLength;
	std::vector<uint8_t> instructions; 
	std::vector<uint8_t> flags;
	std::vector<short> xCoords; //apparently this can also be a uint8_t but we'll see
	std::vector<short> yCoords;
	std::vector<uint16_t> endPtsOfCountours;
};
//for later use
struct comp_glyf : glyf {

};



struct glyph_table {
	std::vector<simp_glyf> simple_glyphs;
	std::vector<comp_glyf> compound_glyphs;
	bool overlap_simple = false;
};
// read glyf table
glyph_table readGlyfs(char* c, int offset, int length, std::vector<loca> locas) {
	glyph_table table;
	for (size_t i = 0; i < locas.size(); i++) {
		char* m = c + offset + locas[i].offset;
		short* s = (short*)m;
		glyf g;
		g.c = locas[i].c;
		g.numberOfContours = SwapTwoBytes(*s);
		s++;
		g.xMin = SwapTwoBytes(*s);
		s++;
		g.yMin = SwapTwoBytes(*s);
		s++;
		g.xMax = SwapTwoBytes(*s);
		s++;
		g.yMax = SwapTwoBytes(*s);
		s++;
		if (g.numberOfContours >= 0) {
			//simple glyph
			simp_glyf sg;
			sg.numberOfContours = g.numberOfContours;
			sg.xMin = g.xMin;
			sg.yMin = g.yMin;
			sg.xMax = g.xMax;
			sg.yMax = g.yMax;
			sg.c = g.c;
			//now we read endpts of countours
			uint16_t* t = (uint16_t*)s;
			for (int j = 0; j < sg.numberOfContours; j++) {
				sg.endPtsOfCountours.push_back(SwapTwoBytes(*t));
				t++;
			}
			//instructions now, can't believe I forgot to swap this smh, like an hour wasted 
			sg.instructionLength = SwapTwoBytes(*t);
			t++;
			uint8_t* d = (uint8_t*)t;
			//don't have to swap 
			for (int j = 0; j < sg.instructionLength; j++) {
				sg.instructions.push_back(*d);
				d++;
			}
			//now flags
			int last_index = sg.endPtsOfCountours[sg.numberOfContours - 1];
			for (int j = 0; j < (last_index + 1); j++) {
				if (j == 0) {
					if (((*d) & OVERLAP_SIMPLE) != 0) {
						table.overlap_simple = true;
					}
				}
				sg.flags.push_back(*d);
				d++;
				if (((sg.flags[j] & REPEAT_FLAG) != 0)) {
					uint8_t repeat_count = *d;
					while (repeat_count-- > 0) {
						j++;
						sg.flags.push_back(sg.flags[j - 1]);
					}
					d++;
				}
			}
			//have to swap these
			//http://stevehanov.ca/blog/?id=143
			//xcoords
			short prev_coord = 0;
			short cur_coord = 0;
			s = (short*)d;
			for (int j = 0; j < (last_index + 1); j++) {
				//fuck ur combined flag bitch
				//int flag_combined = ((getnthBit(sg.flags[j], 1) << 1) | (getnthBit(sg.flags[j], 4)));
				bool dor = false;
				if ((sg.flags[j] & X_SHORT_VECTOR) != 0) {
					//one byte
					uint8_t temp = *d;
					d++;
					short out = temp;
					if ((sg.flags[j] & X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) != 16) {
						out *= -1;
					}
					cur_coord = out + prev_coord;
				}
				else {
					//two byte
					if ((sg.flags[j] & X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) != 0) {
						//same as previous
						cur_coord = prev_coord;
						//dor = true;
					}
					else {
						short* ss = (short*)d;
						short out = SwapTwoBytes(*ss);
						d += 2;
						//signed 16 bit delta vector, ie change in x
						cur_coord = out + prev_coord;

					}
					
				}
				//(dor) ? sg.xCoords.push_back(prev_coord) : sg.xCoords.push_back(cur_coord + prev_coord);
				sg.xCoords.push_back(cur_coord);
				prev_coord = cur_coord;
				
			}
			//ycoords
			prev_coord = 0;
			cur_coord = 0;
			for (int j = 0; j < (last_index + 1); j++) {
				bool dor = false;
				if ((sg.flags[j] & Y_SHORT_VECTOR) != 0) {
					//one byte
					uint8_t temp = *d;
					d++;
					short out = temp;
					if ((sg.flags[j] & Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) != 32) {
						out *= -1;
					}
					cur_coord = out + prev_coord;
				}
				else {
					//two byte
					if ((sg.flags[j] & Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) != 0) {
						//same as previous
						cur_coord = prev_coord;
						//dor = true;
					}
					else {
						short* ss = (short*)d;
						short out = SwapTwoBytes(*ss);
						d += 2;
						//signed 16 bit delta vector, ie change in x
						cur_coord = out + prev_coord;

					}

				}
				//(dor) ? sg.xCoords.push_back(prev_coord) : sg.xCoords.push_back(cur_coord + prev_coord);
				sg.yCoords.push_back(cur_coord);
				prev_coord = cur_coord;
			}
			table.simple_glyphs.push_back(sg);
		}
		else {
			// https://learn.microsoft.com/en-us/typography/opentype/spec/glyf
			//compound glyph do nothing for now
			glyf g;
			uint16_t flags;
			do {
				flags = SwapTwoBytes(*s);
				s++;
				uint16_t glyphIndex = SwapTwoBytes(*s);
				s++;
				if (flags & ARG_1_AND_2_ARE_WORDS) {

				} else {

				}
				if ( flags & WE_HAVE_A_SCALE ) {
					
				} else if ( flags & WE_HAVE_AN_X_AND_Y_SCALE ) {
					
				} else if ( flags & WE_HAVE_A_TWO_BY_TWO ) {
					
				}
			} while (flags & MORE_COMPONENTS);
			if (flags & WE_HAVE_INSTRUCTIONS) {
				
			}
		}

	}
	return table;
}

// find table in directory
table_dir* findTable(std::string table, Font_dir* directory) {
	for (size_t i = 0; i < directory->table.size(); i++) {
		if (directory->table[i].t.compare(table) == 0) {
			return &directory->table[i];
		}
	}
	return nullptr;
}
// tesslate bezier curve into line segments
void tesslateBezier(std::vector<vec2>& points, vec2 p1, vec2 p2, vec2 p3, int subdiv) {
	float step = 1.0f / subdiv;
	float lx = 0, ly = 0;
	for (int i = 0; i <= subdiv; i++) {
		float t = i * step;
		float t1 = (1.0f - t);
		float t2 = t * t;
		float x = t1 * t1 * p1.x + 2 * t1 * t * p2.x + t2 * p3.x;
		float y = t1 * t1 * p1.y + 2 * t1 * t * p2.y + t2 * p3.y;
		(i == 0) ? lx = x, ly = y : lx = lx, ly = ly;
		points.push_back({ lx, ly });
		points.push_back({ x, y });
		lx = x;
		ly = y;
	}
}


std::vector<Line> generate_edges(std::vector<int>& end_contours, std::vector<vec2>& points) {
	std::vector<Line> lines;
	int j = 0;
	for (size_t i = 0; i < end_contours.size(); i++) {
		int first = j;
		for (; j < end_contours[i] - 1; j++) {
			Line l;
			l.p1.x = points[j].x;
			l.p1.y = points[j].y;
			l.p2.x = points[j + 1].x;
			l.p2.y = points[j + 1].y;
			lines.push_back(l);
		}
		//have to add endpoint of contour and last point of contour as a line, so we can fix any possible gaps in glyphs
		lines.push_back({ { points[first].x, points[first].y}, {points[j].x, points[j].y}});
		j++;
	}
	return lines;
}
struct bezier_point {
	vec2 point;
	bool on_curve;
};

// breakdown bezier curve into line segments
void breakBezier(std::vector<Line>& lines, vec2 p1, vec2 p2, vec2 p3, int subdiv) {
	float step = 1.0f / subdiv;
	float lx = 0, ly = 0;
	for (int i = 0; i <= subdiv; i++) {
		float t = i * step;
		float t1 = (1.0f - t);
		float t2 = t * t;
		float x = t1 * t1 * p1.x + 2 * t1 * t * p2.x + t2 * p3.x;
		float y = t1 * t1 * p1.y + 2 * t1 * t * p2.y + t2 * p3.y;
		(i == 0) ? lx = x, ly = y : lx = lx, ly = ly;
		lines.push_back({ { lx, ly }, { x, y } });
		lx = x;
		ly = y;
	}
}
// have to process contour points that are off in a more meaningful way
std::vector<Line> constructLineSegments (std::vector<bezier_point>& countour_points) {
	std::vector<Line> lines;
	for (size_t i = 0; i < countour_points.size();) {
		size_t j = 0;
		for (j = i + 1; j < countour_points.size() && !countour_points[j].on_curve; j++);
		if (j - i == 1 && i + 1 < countour_points.size()) {
			// simple line
			Line l;
			l.p1 = countour_points[i].point;
			l.p2 = countour_points[i + 1].point;
			lines.push_back(l);
		} else if (j - i > 1) {
			// bezier curves
			for (size_t k = i + 1; k <= j && k < countour_points.size(); k+=2) {
				bezier_point p1 = countour_points[k - 1];
				bezier_point p2 = countour_points[k];
				bezier_point p3;
				if (k + 1 <= j && k + 1 < countour_points.size()) {
					p3 = countour_points[k + 1];
				} else {
					// create midpoint between p2 and p1
					vec2 mid;
					mid.x = (p2.point.x + p1.point.x) / 2.0f;
					mid.y = (p2.point.y + p1.point.y) / 2.0f;
					p3 = { mid, true };
				}
				breakBezier(lines, p1.point, p2.point, p3.point, 5);
			}
		}
		i += (j - i) + 1;
	}
	return lines;
}

void constructGlyphs (Font_dir* directory, gore::Font* f, glyph_table* g_table, hmtx_table* hmtx) {
	for (auto& i : g_table->simple_glyphs) {
		gore::Glyph g;
		g.c = i.c;
		g.xMax = i.xMax;
		g.yMax = i.yMax;
		g.yMin = i.yMin;
		g.xMin = i.xMin;
		g.advanceWidth = hmtx->hMetrics[i.c].advanceWidth;
		g.lsb = hmtx->hMetrics[i.c].lsb;
		int32_t k = 0;
		std::vector<vec2> points;
		for (int32_t j = 0; j < i.numberOfContours; j++) {
			int32_t generated_points_start_index = (points.size() > 0) ? (int32_t)points.size() - 1 : 0;
			int32_t contour_start_index = k;
			std::vector<bezier_point> contour_points;
			for (int32_t c = 0; k <= i.endPtsOfCountours[j]; k++, c++) {
				// the x and y coords are diffs from the last points
				float x = i.xCoords[k];
				float y = i.yCoords[k];
				contour_points.push_back({ { x, y }, (i.flags[k] & ON_CURVE_POINT) != 0 });
			}
			//now we have all contour points, we can generate the bezier curves
			std::vector<Line> contour_lines = constructLineSegments(contour_points);
			//append to glyph lines
			g.contours.insert(g.contours.end(), contour_lines.begin(), contour_lines.end());
			k = i.endPtsOfCountours[j] + 1;

		}
		f->glyphs.push_back(g);
	}
}

void readDirectorys(Font_dir* directory, gore::Font* f, char* c, uint16_t start, uint16_t end) {
	//getting directorys in order we need them
	cmap c_map;
	TTFHeader header;
	std::vector<loca> locas;
	glyph_table g_table;
	table_dir* tab = nullptr;
	tab = findTable("cmap", directory);
	c_map = readCmap(c, tab->offset, tab->length, start, end);
	tab = findTable("head", directory);
	header = readHead(c, tab->offset, tab->length);
	tab = findTable("loca", directory);
	locas = readLoca(c, tab->offset, tab->length, header.indexToLocFormat, &c_map);
	tab = findTable("hhea", directory);
	hhea_table hhea = readHheaTable(c, tab->offset, tab->length);
    tab = findTable("hmtx", directory);
	hmtx_table hmtx = readHmtxTable(c, tab->offset, tab->length, hhea.numberOfHMetrics, locas.size());
	tab = findTable("glyf", directory);
	g_table = readGlyfs(c, tab->offset, tab->length, locas);
    tab = findTable("vmtx", directory);
    tab = findTable("vhea", directory);
    tab = findTable("gpos", directory);
    tab = findTable("gdef", directory);
    tab = findTable("kern", directory);
	f->unitsPerEm = header.uintsPerEm;
	f->overlap_simple = g_table.overlap_simple;
	constructGlyphs(directory, f, &g_table, &hmtx);
	//don't want to store pointers to anything in gore::Font file
	//https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization, 2.2
	/*for (auto& i : g_table.simple_glyphs) {
		gore::Glyph g;
		g.c = i.c;
		g.xMax = i.xMax;
		g.yMax = i.yMax;
		g.yMin = i.yMin;
		g.xMin = i.xMin;
		g.advanceWidth = hmtx.hMetrics[i.c].advanceWidth;
		g.lsb = hmtx.hMetrics[i.c].lsb;
		int k = 0;
		std::vector<vec2> points;
		std::vector<int> end_contours;
		for (int j = 0; j < i.numberOfContours; j++) {
			int generated_points_start_index = (int)points.size() - 1;
			if (generated_points_start_index < 0) {
				generated_points_start_index = 0;
			}
			int contour_start_index = k;
			bool contour_start = true;
			bool contour_started_off = false;
			//this was the issue
			for (; k <= i.endPtsOfCountours[j]; k++) {
				int contour_len = i.endPtsOfCountours[j] - contour_start_index + 1;
				int cur_index = k;
				int next_index = (k + 1 - contour_start_index) % contour_len + contour_start_index;

				float x = i.xCoords[k];
				float y = i.yCoords[k];
				if ((i.flags[k] & ON_CURVE_POINT) != 0) {
					size_t p3_in = k + 1;
					if (k == i.endPtsOfCountours[j]) {
						p3_in = 0;
					}
					vec2 p1 = { (float)i.xCoords[k], (float)i.yCoords[k] };
					vec2 p2 = { (float)i.xCoords[p3_in], (float)i.yCoords[p3_in] };
					vec2 p3;
					p3.x = p2.x + (p1.x - p2.x) / 2.0f;
					p3.y = p2.y + (p1.y - p2.y) / 2.0f;
					points.push_back({ x, y});
				}
				else{
					//if this is the first contour point
					if (contour_start) {
						contour_started_off = true; 
						//next point is on curve
						if ((i.flags[next_index] & ON_CURVE_POINT) != 0) {
							points.push_back({ (float)i.xCoords[next_index], (float)i.yCoords[next_index] });
							k++;
							continue;
						}
						x = x + (i.xCoords[next_index] - x) / 2.0f;
						y = y + (i.yCoords[next_index] - y) / 2.0f;
						points.push_back({ x, y });
						
					}
					
					vec2 p1 = points[points.size() - 1];
					vec2 p2 = { (float)x, (float)y };
					vec2 p3 = { (float)i.xCoords[next_index], (float)i.yCoords[next_index] };
					//get the middle point between p1 and p3
					if ((i.flags[next_index] & ON_CURVE_POINT) != 0) {
						p3.x = p2.x + (p3.x - p2.x) / 2.0f;
						p3.y = p2.y + (p3.y - p2.y) / 2.0f;
					}
					else {
						k++;
					}
					//generate points
					tesslateBezier(points, p1, p2, p3, 2);
				}
				contour_start = false;
			}
			if ((i.flags[k - 1] & ON_CURVE_POINT) != 0) {
				points.push_back({ (float)i.xCoords[contour_start_index] , (float)i.yCoords[contour_start_index] });
			}
			if (contour_started_off) {
				vec2 p1 = points[points.size() - 1];
				vec2 p2;
				p2.x = (float)i.xCoords[contour_start_index];
				p2.y = (float)i.yCoords[contour_start_index];
				vec2 p3 = points[generated_points_start_index];

				tesslateBezier(points, p1, p2, p3, 2);
			}
			end_contours.push_back((int)points.size());
		}
		g.contours = generate_edges(end_contours, points);
		f->glyphs.push_back(g);
	}*/

}


//https://docs.fileformat.com/font/ttf/
//https://handmade.network/forums/articles/t/7330-implementing_a_font_reader_and_rasterizer_from_scratch%252C_part_1__ttf_font_reader. part 12
//https://tchayen.github.io/posts/ttf-file-parsing
// https://github.com/RazrFalcon/ttf-parser
//https://learn.microsoft.com/en-us/typography/opentype/spec/ttch01
// http://stevehanov.ca/blog/?id=143
//https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6.html
// https://learn.microsoft.com/en-us/typography/opentype/spec/
// https://tchayen.github.io/posts/ttf-file-parsing
// https://fontdrop.info/
//big endian so characters will be reversed to me
//start and end variables are the start of characters you want to load and end is the last character to load
gore::Font gore::FontRenderer::loadFont(std::string file, uint16_t start, uint16_t end) {
	std::ifstream f;
	f.open(file.c_str(), std::ios::binary);
	if (!f.is_open()) {
		std::cout << "Failed to open font file: " << file << std::endl;
		return gore::Font();
	}
	//read the file into memory
	std::stringstream stream;
	stream << f.rdbuf();
	std::string st = stream.str();
	char* c = (char*)st.c_str();
	f.close();
	//read the gore::Font directory
	Font_dir directory;
	read_offset_subtable(c, &directory.off_sub);
	c += 12;
	read_table_directory(c,  directory.table, directory.off_sub.numTables);
	c = (char*)st.c_str(); //reset to begining to get offset easier
	//now we read all of the directorys we need to
	gore::Font Font;
	Font.name = file;
	Font.overlap_simple = false;
	readDirectorys(&directory, &Font, c, start, end);

	return Font;
}
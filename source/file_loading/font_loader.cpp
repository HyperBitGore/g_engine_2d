#include "font_loader.hpp"
#include "file_reader.hpp"

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

// GPOS lookup table types
#define GPOS_SINGLE_ADJUSTMENT 0x1
#define GPOS_PAIR_ADJUSTMENT 0x2
#define GPOS_CURSIVE_ATTACHMENT 0x3
#define GPOS_MARK_TO_BASE_ATTACHMENT 0x4
#define GPOS_MARK_TO_LIGATURE_ATTACHMENT 0x5
#define GPOS_MARK_TO_MARK_ATTACHMENT 0x6
#define GPOS_CONTEXTUAL_POSITIONING 0x7
#define GPOS_CHAINED_CONTEXTS_POSITIONING 0x8
#define GPOS_POSITIONING_EXTENSION 0x9

#define VALUE_FORMAT_X_PLACEMENT 0x0001
#define VALUE_FORMAT_Y_PLACEMENT 0x0002
#define VALUE_FORMAT_X_ADVANCE 0x0004
#define VALUE_FORMAT_Y_ADVANCE 0x0008
#define VALUE_FORMAT_X_PLACEMENT_DEVICE 0x0010
#define VALUE_FORMAT_Y_PLACEMENT_DEVICE 0x0020
#define VALUE_FORMAT_X_ADVANCE_DEVICE 0x0040
#define VALUE_FORMAT_Y_ADVANCE_DEVICE 0x0080


inline int getnthBit(short number, int n) {
	return (number >> n) & 1;
}
inline int getnthBit(uint8_t number, int n) {
	return (number >> n) & 1;
}


inline uint8_t swap1Byte(uint8_t n) {
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
void read_offset_subtable(FileReader* fr, off_subtable* table){
	table->scaler_type = fr->readFourBytes(true);
	table->numTables = fr->readTwoBytes(true);
	table->searchRange = fr->readTwoBytes(true);
	table->entrySelector = fr->readTwoBytes(true);
	table->rangeShift = fr->readTwoBytes(true);
}

void read_table_directory(FileReader* fr, std::vector<table_dir>& table, int tbl_size) {
	for (int i = 0; i < tbl_size; i++) {
		table_dir dir;
		dir.t = "";
		for (int j = 0; j < 4; j++) {
			char c = fr->readOneByte();
			dir.t.push_back(c);
		}
		fr->moveHeadBack(4);
		dir.tag = fr->readFourBytes(true);
		dir.checksum = fr->readFourBytes(true);
		dir.offset = fr->readFourBytes(true);
		dir.length = fr->readFourBytes(true);
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
cmap readCmap(FileReader* fr, int offset, int length, uint16_t start, uint16_t end) {
	cmap map;
	fr->moveHeadForward(offset);
    uint32_t start_offset = fr->getOffset();
	char* m = fr->getHead();
	uint32_t start_off = fr->getOffset();
	map.version = fr->readTwoBytes(true);
	map.numTables = fr->readTwoBytes(true);
	//now we read the subtables
	for (int i = 0; i < (int)map.numTables; i++) {
		cmap_table table;
		table.platformID = fr->readTwoBytes(true);
		table.platformSpecificID = fr->readTwoBytes(true);
		table.offset = fr->readFourBytes(true);
		map.tables.push_back(table); //offset is from start of cmap
    }
    for (auto& i : map.tables) {
        fr->setHead(start_offset);
        fr->moveHeadForward(i.offset);
        uint16_t format = fr->readTwoBytes(true);
		switch (format) {
		case 4:
			//most common
			readFormat4(m, &i, start, end);
			break;
		case 0:
			readFormat0(m, &i, start, end);
			break;
		case 2:
			std::cout << "Unsupported cmap format; Format 2;" << std::endl;
			//readFormat2(m, &map.tables[i], start, end);
			break;
		case 6:
			readFormat6(m, &i, start, end);
			break;
		case 8:
			std::cout << "Unsupported cmap format; Format 8;" << std::endl;
			//readFormat8(m, &map.tables[i], start, end);
			break;
		case 10:
			readFormat10(m, &i, start, end);
			break;
		case 12:
			//most common
			readFormat12(m, &i, start, end);
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
	uint16_t index;
};



//read the table when it is the 16 bit version
inline uint32_t readLoca16(char* s, uint16_t index) {
	uint16_t* d = ((uint16_t*)s + index);
	return SwapTwoBytes(*d) * 2;
}
//reading the table when it is the 32 bit version
inline uint32_t readLoca32(char* s, uint16_t index) {
	uint32_t* d = (uint32_t*)s + index;
	return SwapFourBytes(*d);
}

std::vector<loca> readLoca(FileReader* fr, int offset, int length, uint16_t format, cmap* map) {
	fr->moveHeadForward(offset);
	std::vector<loca> locas;
	int index = 0;
	for (size_t i = 0; i < map->tables.size(); i++) {
		if (map->tables[i].platformSpecificID == 3) {
			index = (int)i;
			break;
		}
	}
	for (size_t i = 0; i < map->tables[index].indexs.size(); i++) {
		loca l;
		l.c = map->tables[index].indexs[i].c;
		l.index = map->tables[index].indexs[i].index;
		l.offset = (format == 0) ? (readLoca16(fr->getHead(), map->tables[index].indexs[i].index)) : (readLoca32(fr->getHead(), map->tables[index].indexs[i].index));
		locas.push_back(l);
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



TTFHeader readHead(FileReader* fr, int offset, int length) {
	TTFHeader head;
	fr->setHead(offset);
	head.version = fr->readFourBytes(true); //can't swap the bytes on a double for some reason
	head.FontRevision = fr->readFourBytes(true); //values will be wrong for the top two since supposed to be fixed
	head.checkSumAdjustment = fr->readFourBytes(true);
	head.magicNumber = fr->readFourBytes(true);
	head.flags = fr->readTwoBytes(false); //don't swap this since the bits need to be the same
	head.uintsPerEm = fr->readTwoBytes(true);
	time_t* p = (time_t*)fr->getHead();
	head.created = SwapEightBytes(*p);
	fr->moveHeadForward(sizeof(time_t));
	p = (time_t*)fr->getHead();
	head.modified = SwapEightBytes(*p);
	fr->moveHeadForward(sizeof(time_t));
	head.xMin = fr->readTwoBytes(true);
	head.yMin = fr->readTwoBytes(true);
	head.xMax = fr->readTwoBytes(true);
	head.yMax = fr->readTwoBytes(true);
	head.macStyle = fr->readTwoBytes(false); //don't swap cause need same bits for flags
	head.lowestRecPPEM = fr->readTwoBytes(true);
	head.FontDirectionHint = fr->readTwoBytes(true);; 
	head.indexToLocFormat =fr->readTwoBytes(true);;
	head.glyphDataFormat = fr->readTwoBytes(true);;
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

hhea_table readHheaTable(FileReader* fr, int offset, int length) {
	hhea_table h;
	fr->moveHeadForward(offset);
	h.majorVersion = fr->readTwoBytes(true);
	h.minorVersion = fr->readTwoBytes(true);
	h.ascender = fr->readTwoBytes(true);
	h.descender = fr->readTwoBytes(true);
	h.lineGap = fr->readTwoBytes(true);
	h.advanceWidthMax = fr->readTwoBytes(true);
	h.minLeftSideBearing = fr->readTwoBytes(true);
	h.minRightSideBearing = fr->readTwoBytes(true);
	h.xMaxExtent = fr->readTwoBytes(true);
	h.caretSlopeRise = fr->readTwoBytes(true);
	h.caretSlopeRun = fr->readTwoBytes(true);
	h.caretOffset = fr->readTwoBytes(true);
	fr->moveHeadForward(4 * sizeof(uint16_t)); // skipping reserved
	h.metricDataFormat = fr->readTwoBytes(true);
	h.numberOfHMetrics = fr->readTwoBytes(true);

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

hmtx_table readHmtxTable(FileReader* fr, int offset, int length, uint16_t numHMetrics, size_t numGlyphs) {
	hmtx_table h;
	fr->moveHeadForward(offset);
	for (size_t i = 0; i < numHMetrics; i++) {
		long_hor_metric hm;
		hm.advanceWidth = fr->readTwoBytes(true);
		hm.lsb = fr->readTwoBytes(true);
		h.hMetrics.push_back(hm);
	}
	//now we read the left side bearings
	for (size_t i = numHMetrics; i < numGlyphs; i++) {
		h.leftSideBearings.push_back(fr->readTwoBytes(true));
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
	uint16_t glyf_index;
};



struct simp_glyf : glyf {
	uint16_t instructionLength;
	int16_t xPos = 0;
	int16_t yPos = 0;
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
glyph_table readGlyfs(FileReader* fr, int offset, int length, std::vector<loca> locas) {
	glyph_table table;
	for (size_t i = 0; i < locas.size(); i++) {
		fr->resetHead();
		fr->moveHeadForward(offset + locas[i].offset);
		glyf g;
		g.c = locas[i].c;
		g.numberOfContours = fr->readTwoBytes(true);
		g.xMin = fr->readTwoBytes(true);
		g.yMin = fr->readTwoBytes(true);
		g.xMax = fr->readTwoBytes(true);
		g.yMax = fr->readTwoBytes(true);
		g.glyf_index = locas[i].index;
		if (g.numberOfContours >= 0) {
			//simple glyph
			simp_glyf sg;
			sg.numberOfContours = g.numberOfContours;
			sg.xMin = g.xMin;
			sg.yMin = g.yMin;
			sg.xMax = g.xMax;
			sg.yMax = g.yMax;
			sg.c = g.c;
			sg.glyf_index = g.glyf_index;
			//now we read endpts of countours
			for (int j = 0; j < sg.numberOfContours; j++) {
				sg.endPtsOfCountours.push_back(fr->readTwoBytes(true));
			}
			//instructions now, can't believe I forgot to swap this smh, like an hour wasted 
			sg.instructionLength = fr->readTwoBytes(true);
			//don't have to swap 
			for (int j = 0; j < sg.instructionLength; j++) {
				sg.instructions.push_back(fr->readOneByte());
			}
			//now flags
			int last_index = sg.endPtsOfCountours[sg.numberOfContours - 1];
			for (int j = 0; j < (last_index + 1); j++) {
				uint8_t flag = fr->readOneByte();
				if (j == 0) {
					if ((flag & OVERLAP_SIMPLE) != 0) {
						table.overlap_simple = true;
					}
				}
				sg.flags.push_back(flag);
				if (((sg.flags[j] & REPEAT_FLAG) != 0)) {
					uint8_t repeat_count = fr->readOneByte();
					while (repeat_count-- > 0) {
						j++;
						sg.flags.push_back(sg.flags[j - 1]);
					}
				}
			}
			//have to swap these
			//http://stevehanov.ca/blog/?id=143
			//xcoords
			short prev_coord = 0;
			short cur_coord = 0;
			for (int j = 0; j < (last_index + 1); j++) {
				//fuck ur combined flag bitch
				//int flag_combined = ((getnthBit(sg.flags[j], 1) << 1) | (getnthBit(sg.flags[j], 4)));
				bool dor = false;
				if ((sg.flags[j] & X_SHORT_VECTOR) != 0) {
					//one byte
					uint8_t temp = fr->readOneByte();
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
						short out = fr->readTwoBytes(true);
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
					uint8_t temp = fr->readOneByte();
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
						short out = fr->readTwoBytes(true);
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
				flags = fr->readTwoBytes(true);
				uint16_t glyphIndex = fr->readTwoBytes(true);
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

// don't need original version since can just ignore the featureVariationsOffset when 1.0
PREVENT_PACKING_STRUCT gpos_1_1 {
	uint16_t majorVersion;
	uint16_t minorVersion;
	uint16_t scriptListOffset;
	uint16_t featureListOffset;
	uint16_t lookupListOffset;
	uint32_t featureVariationsOffset;
};
END_PACKING_STRUCT

PREVENT_PACKING_STRUCT FeatureRecord {
	uint32_t featureTag; // 4-byte feature identification tag.
	uint16_t featureOffset; // Offset to Feature table, from beginning of FeatureList.
};
END_PACKING_STRUCT

PREVENT_PACKING_STRUCT LookupTable {
	uint16_t lookupType;
	uint16_t lookupFlag;
	uint16_t subTableCount;
};
END_PACKING_STRUCT

PREVENT_PACKING_STRUCT SequenceLookup {
	uint16_t sequenceIndex; // Index (zero-based) into the input glyph sequence.
	uint16_t lookupListIndex; // Index (zero-based) into the LookupList.
};
END_PACKING_STRUCT

PREVENT_PACKING_STRUCT SinglePosFormat1 {
	uint16_t format;
	uint16_t coverageOffset;
	uint16_t valueFormat;
};
END_PACKING_STRUCT

PREVENT_PACKING_STRUCT SinglePosFormat2 {
	uint16_t format;
	uint16_t coverageOffset;
	uint16_t valueFormat;
	uint16_t valueCount;
};
END_PACKING_STRUCT

PREVENT_PACKING_STRUCT RangeRecord {
	uint16_t startGlyphID;
	uint16_t endGlyphID;
	uint16_t startCoverageIndex;
};
END_PACKING_STRUCT

PREVENT_PACKING_STRUCT MarkBasePosFormat1 {
    uint16_t format;
    uint16_t markCoverageOffset;
    uint16_t baseCoverageOffset;
    uint16_t markClassCount;
    uint16_t markArrayOffset;
    uint16_t baseArrayOffset;
};
END_PACKING_STRUCT

struct ChainedSequenceRule {
	std::vector<uint16_t> backtrackSequence;
	std::vector<uint16_t> inputSequence;
	std::vector<uint16_t> lookaheadSequence;
	std::vector<SequenceLookup> seqLookupRecords;
};

std::vector<uint16_t> readCoverageTable (char* start) {
	std::vector<uint16_t> glyphs;
	uint16_t format = SwapTwoBytes(*(uint16_t*)(start));
	uint16_t count = SwapTwoBytes(*(uint16_t*)(start + 2)); // either glyph count or range count
	if (format == 1) {
		for (size_t i = 0; i < count; i++) {
			uint16_t glyf = SwapTwoBytes(*(uint16_t*)(start + 4 + (i * sizeof(uint16_t))));
			glyphs.push_back(glyf);
		}
	} else if (format == 2) {
		for (size_t i = 0; i < count; i++) {
			RangeRecord record = *(RangeRecord*)(start + 4 + (i * sizeof(RangeRecord)));
			record.endGlyphID = SwapTwoBytes(record.endGlyphID);
			record.startCoverageIndex = SwapTwoBytes(record.startCoverageIndex);
			record.startGlyphID = SwapTwoBytes(record.startGlyphID);
			for (uint16_t j = record.startGlyphID; j <= record.endGlyphID; j++) {
				glyphs.push_back(j);
			}
		}
	}

	return glyphs;
}

uint32_t readValueRecord (char* start, uint16_t format, glyph_table* glyf_table, std::vector<uint16_t> glyphs) {
	uint32_t offset = 0;
	if ((format & VALUE_FORMAT_X_PLACEMENT) != 0) {
		int16_t xPlacement = SwapTwoBytes(*(int16_t*)(start + offset));
		for (auto& i : glyphs) {
			for (auto& j : glyf_table->simple_glyphs) {
				if (i == j.c) {
					for (size_t k = 0; k < j.xCoords.size(); k++) {
						j.xPos += xPlacement;
					}
				}
			}
		}
		offset += sizeof(int16_t);
	}
	if ((format & VALUE_FORMAT_Y_PLACEMENT) != 0) {
		int16_t yPlacement = SwapTwoBytes(*(int16_t*)(start + offset)); 
		for (auto& i : glyphs) {
			for (auto& j : glyf_table->simple_glyphs) {
				if (i == j.c) {
					for (size_t k = 0; k < j.xCoords.size(); k++) {
						j.yPos += yPlacement;
					}
				}
			}
		}
		offset += sizeof(int16_t);
	}
	if ((format & VALUE_FORMAT_X_ADVANCE) != 0) {
		int16_t xAdvance = *(int16_t*)(start + offset); 
		for (auto& i : glyphs) {
			for (auto& j : glyf_table->simple_glyphs) {
				if (i == j.c) {

				}
			}
		}
		offset += sizeof(int16_t);
	}
	if ((format & VALUE_FORMAT_Y_ADVANCE) != 0) {
		int16_t yAdvance = *(int16_t*)(start + offset); 
		for (auto& i : glyphs) {
			for (auto& j : glyf_table->simple_glyphs) {
				if (i == j.c) {
					
				}
			}
		}
		offset += sizeof(int16_t);
	}
	if ((format & VALUE_FORMAT_X_PLACEMENT_DEVICE) != 0) {
		int16_t xPlaDeviceOffset = *(uint16_t*)(start + offset); 
		for (auto& i : glyphs) {
			for (auto& j : glyf_table->simple_glyphs) {
				if (i == j.c) {
					
				}
			}
		}
		offset += sizeof(uint16_t);
	}
	if ((format & VALUE_FORMAT_Y_PLACEMENT_DEVICE) != 0) {
		int16_t yPlaDeviceOffset = *(uint16_t*)(start + offset); 
		for (auto& i : glyphs) {
			for (auto& j : glyf_table->simple_glyphs) {
				if (i == j.c) {
					
				}
			}
		}
		offset += sizeof(uint16_t);
	}
	if ((format & VALUE_FORMAT_X_ADVANCE_DEVICE) != 0) {
		int16_t xAdvDeviceOffset = *(uint16_t*)(start + offset); 
		for (auto& i : glyphs) {
			for (auto& j : glyf_table->simple_glyphs) {
				if (i == j.c) {
					
				}
			}
		}
		offset += sizeof(uint16_t);
	}
	if ((format & VALUE_FORMAT_Y_ADVANCE_DEVICE) != 0) {
		int16_t yAdvDeviceOffset = *(uint16_t*)(start + offset); 
		for (auto& i : glyphs) {
			for (auto& j : glyf_table->simple_glyphs) {
				if (i == j.c) {
					
				}
			}
		}
		offset += sizeof(uint16_t);
	}
	return offset;
}

// https://learn.microsoft.com/en-us/typography/opentype/spec/gpos
// should just directly modify glyf data, instead of returning anything
// would be easier tbh
void readGpos (FileReader* fr, int32_t offset, int32_t length, glyph_table* glyf_table) {
	fr->moveHeadForward(offset);
	uint32_t start_offset = fr->getOffset();
	gpos_1_1 gposheader = *(gpos_1_1*)(fr->getHead());
	gposheader.majorVersion = SwapTwoBytes(gposheader.majorVersion);
	gposheader.minorVersion = SwapTwoBytes(gposheader.minorVersion);
	gposheader.scriptListOffset = SwapTwoBytes(gposheader.scriptListOffset);
	gposheader.featureListOffset = SwapTwoBytes(gposheader.featureListOffset);
	gposheader.lookupListOffset = SwapTwoBytes(gposheader.lookupListOffset);
	gposheader.featureVariationsOffset = SwapFourBytes(gposheader.featureVariationsOffset);

	// parsing feature list
	fr->moveHeadForward(gposheader.featureListOffset);
	uint16_t featureCount = fr->readTwoBytes(true);
	std::vector<FeatureRecord> feature_records;
	for (size_t i = 0; i < featureCount; i++) {
		FeatureRecord feature_record = *(FeatureRecord*)(fr->getHead());
		feature_record.featureTag = SwapFourBytes(feature_record.featureTag);
		feature_record.featureOffset = SwapTwoBytes(feature_record.featureOffset);
		feature_records.push_back(feature_record);
		fr->moveHeadForward(sizeof(FeatureRecord));
	}
	// parsing lookup list
	fr->setHead(start_offset);
	fr->moveHeadForward(gposheader.lookupListOffset);
	uint32_t start_lookup_table = fr->getOffset();
	uint16_t lookup_count = fr->readTwoBytes(true);
	std::vector<ChainedSequenceRule> chainedSeqRules;
	std::vector<uint16_t> looktable_offsets;
	for (size_t i = 0; i < lookup_count; i++) {
		uint16_t lookup_offset = fr->readTwoBytes(true);
		looktable_offsets.push_back(lookup_offset);
	}
	for (auto& lookup_offset : looktable_offsets) {
		fr->setHead(start_lookup_table);
		fr->moveHeadForward(lookup_offset);
		LookupTable lt = *(LookupTable*)(fr->getHead());
		lt.lookupFlag = SwapTwoBytes(lt.lookupFlag);
		lt.lookupType = SwapTwoBytes(lt.lookupType);
		lt.subTableCount = SwapTwoBytes(lt.subTableCount);
		std::vector<uint16_t> subtableOffsets;
		fr->moveHeadForward(sizeof(LookupTable));
		for (size_t j = 0; j < lt.subTableCount; j++) {
			// read subtable offsets
			subtableOffsets.push_back(fr->readTwoBytes(true));
		}
		for (size_t j = 0; j < subtableOffsets.size(); j++) {
			fr->setHead(start_lookup_table);
			fr->moveHeadForward(lookup_offset);
			fr->moveHeadForward(subtableOffsets[j]);
			if (lt.lookupType == GPOS_POSITIONING_EXTENSION) {
				uint16_t format = fr->readTwoBytes(true);
				uint16_t lookup_type = fr->readTwoBytes(true);
				uint32_t extension_offset = fr->readFourBytes(true);
				lt.lookupType = lookup_type;
				fr->moveHeadForward(extension_offset - 8);
			}
			switch (lt.lookupType) {
				case GPOS_SINGLE_ADJUSTMENT:
					{
						SinglePosFormat2 header = (*(SinglePosFormat2*)(fr->getHead()));
						header.format = SwapTwoBytes(header.format);
						header.coverageOffset = SwapTwoBytes(header.coverageOffset);
						header.valueCount = SwapTwoBytes(header.valueCount);
						header.valueFormat = SwapTwoBytes(header.valueFormat);
						// read the coverage table
						std::vector<uint16_t> glyphs = readCoverageTable(fr->getHead() + header.coverageOffset);
						if (header.format == 1) {
							uint32_t offset = readValueRecord(fr->getHead() + sizeof(SinglePosFormat1), header.valueFormat, glyf_table, glyphs);
						} else if (header.format == 2) {
							// format 2 could be assumed, but what if extended? Better to explicitly check here
							uint32_t offset = 0;
							for (size_t i = 0; i < header.valueCount; i++) {
								int16_t recOffset = readValueRecord(fr->getHead() + sizeof(SinglePosFormat2) + (offset), header.valueFormat, glyf_table, glyphs);
								offset += recOffset;
							}
						}
					}
				break;
				case GPOS_PAIR_ADJUSTMENT:
					// not supporting this yet
				break;
				case GPOS_CURSIVE_ATTACHMENT:
					// not supporting this yet
				break;
				case GPOS_MARK_TO_BASE_ATTACHMENT:
					{
						MarkBasePosFormat1 header = *(MarkBasePosFormat1*)(fr->getHead());
                        header.format = SwapTwoBytes(header.format);
                        header.markCoverageOffset = SwapTwoBytes(header.markCoverageOffset);
                        header.baseCoverageOffset = SwapTwoBytes(header.baseCoverageOffset);
                        header.markClassCount = SwapTwoBytes(header.markClassCount);
                        header.markArrayOffset = SwapTwoBytes(header.markArrayOffset);
                        header.baseArrayOffset = SwapTwoBytes(header.baseArrayOffset);
                        

					}
				break;
				case GPOS_MARK_TO_LIGATURE_ATTACHMENT:
					// not supporting this yet
				break;
				case GPOS_MARK_TO_MARK_ATTACHMENT:
					// not supporting this yet
				break;
				case GPOS_CONTEXTUAL_POSITIONING:
					// not supporting this yet
				break;
				case GPOS_CHAINED_CONTEXTS_POSITIONING:
					{
						uint16_t format = fr->readTwoBytes(true);
						uint16_t coverageOffset = fr->readTwoBytes(true);
						uint16_t chainedSeqRuleCount = fr->readTwoBytes(true);
						// process the ChainedSequenceRuleSet table
						/*for (size_t j = 0; j < chainedSeqRuleCount; j++) {
							uint16_t chainedSeqRuleSetOffset = SwapTwoBytes(*(uint16_t*)(cur_off + 6 + (j * sizeof(uint16_t))));
							uint16_t count = SwapTwoBytes(*(uint16_t*)(cur_off + chainedSeqRuleSetOffset));
							for (size_t k = 0; k < count; k++) {
								// process the ChainedSequenceRule table
								uint16_t chainedSeqRuleOffset = SwapTwoBytes(*(uint16_t*)(cur_off + chainedSeqRuleSetOffset + 2 + (k * sizeof(uint16_t))));
								uint16_t backtrackGlyphCount = SwapTwoBytes(*(uint16_t*)(cur_off + chainedSeqRuleSetOffset + chainedSeqRuleOffset));
								std::vector<uint16_t> backtrackSequence;
								// backtrackSequence
								for (size_t l = 0; l < backtrackGlyphCount; l++) {
									uint16_t backtrackGlyphCount = SwapTwoBytes(*(uint16_t*)(cur_off + chainedSeqRuleSetOffset + chainedSeqRuleOffset + 2 + (l * sizeof(uint16_t))));
									backtrackSequence.push_back(backtrackGlyphCount);
								}
								std::vector<uint16_t> inputSequence;
								uint16_t inputGlyphCount = SwapTwoBytes(*(uint16_t*)(cur_off + chainedSeqRuleSetOffset + chainedSeqRuleOffset + 2 + (backtrackGlyphCount * sizeof(uint16_t))));
								for (size_t l = 0; l < inputGlyphCount - 1; l++) {
									inputSequence.push_back(SwapTwoBytes(*(uint16_t*)(cur_off + chainedSeqRuleSetOffset + chainedSeqRuleOffset + 2 + (backtrackGlyphCount * sizeof(uint16_t))) + (l * sizeof(uint16_t))));
								}
								uint16_t lookaheadGlyphCount = SwapTwoBytes(*(uint16_t*)(cur_off + chainedSeqRuleSetOffset + chainedSeqRuleOffset + 2 + (backtrackGlyphCount * sizeof(uint16_t)) + (inputGlyphCount * sizeof(uint16_t))));
								std::vector<uint16_t> lookaheadSequence;
								for (size_t l = 0; l < lookaheadGlyphCount - 1; l++) {
									lookaheadSequence.push_back(SwapTwoBytes(*(uint16_t*)(cur_off + chainedSeqRuleSetOffset + chainedSeqRuleOffset + 2 + (backtrackGlyphCount * sizeof(uint16_t))) + (inputGlyphCount * sizeof(uint16_t)) + (l * sizeof(uint16_t))));
								}
								uint16_t seqLookupCount = SwapTwoBytes(*(uint16_t*)(cur_off + chainedSeqRuleSetOffset + chainedSeqRuleOffset + 2 + (backtrackGlyphCount * sizeof(uint16_t)) + (inputGlyphCount * sizeof(uint16_t)) + (lookaheadGlyphCount * sizeof(uint16_t))));
								std::vector<SequenceLookup> seqLookupRecords;
								// seqLookupRecords
								for (size_t l = 0; l < seqLookupCount; l++) {
									SequenceLookup look = (*(SequenceLookup*)(cur_off + chainedSeqRuleSetOffset + chainedSeqRuleOffset + 2 + (backtrackGlyphCount * sizeof(uint16_t)) + (inputGlyphCount * sizeof(uint16_t)) + (lookaheadGlyphCount * sizeof(uint16_t)) + (l + (sizeof(uint16_t) * 2))));
									look.lookupListIndex = SwapTwoBytes(look.lookupListIndex);
									look.sequenceIndex = SwapTwoBytes(look.sequenceIndex);
									seqLookupRecords.push_back(look);
								}
								ChainedSequenceRule rule = { backtrackSequence, inputSequence, lookaheadSequence, seqLookupRecords};
								chainedSeqRules.push_back(rule);
							}
						}*/
					}
				break;
			}
		}
	}
	// not gonna do these tbh
	// parsing the script list

	// feature variations table
	if (gposheader.majorVersion == 1 && gposheader.minorVersion == 1) {

	}
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
struct bezier_point {
	vec2 point;
	bool on_curve;
};

// breakdown bezier curve into line segments
void breakBezier(std::vector<Line>& lines, vec2 p1, vec2 p2, vec2 p3, int subdiv) {
	float step = 1.0f / subdiv;
	float lx = p1.x, ly = p1.y;
	for (int i = 0; i <= subdiv; i++) {
		float t = i * step;
		float t1 = (1.0f - t);
		float t2 = t * t;
		float x = t1 * t1 * p1.x + 2 * t1 * t * p2.x + t2 * p3.x;
		float y = t1 * t1 * p1.y + 2 * t1 * t * p2.y + t2 * p3.y;
		lines.push_back({ { lx, ly }, { x, y } });
		lx = x;
		ly = y;
	}
}
// have to process contour points that are off in a more meaningful way
std::vector<Line> constructLineSegments (std::vector<bezier_point>& countour_points) {
	for (size_t i = 0; i < countour_points.size(); i++) {
		if (i != 0 && !countour_points[i].on_curve && !countour_points[i - 1].on_curve) {
			vec2 mid;
			mid.x = (countour_points[i - 1].point.x + countour_points[i].point.x) / 2.0f;
			mid.y = (countour_points[i - 1].point.y + countour_points[i].point.y) / 2.0f;
			countour_points.insert(countour_points.begin() + i, {mid, true});
		}
	}
	std::vector<Line> lines;
	vec2 last_on_curve;
	for (size_t i = 0; i < countour_points.size(); i++) {
		bezier_point p1 = countour_points[i];
		bezier_point p2 = countour_points[(i + 1) % countour_points.size()];
		if (p1.on_curve && p2.on_curve) {
			// simple line
			Line l;
			l.p1 = p1.point;
			l.p2 = p2.point;
			lines.push_back(l);
			last_on_curve = p2.point;
		} else if (p1.on_curve && !p2.on_curve) {
			bezier_point p3 = countour_points[(i + 2) % countour_points.size()];
			last_on_curve = p3.point;
			breakBezier(lines, p1.point, p2.point, p3.point, 5);
			i++;
		}
	}
	if (lines.back().p2.x != lines.front().p1.x && lines.back().p2.y != lines.front().p1.y) {
    	lines.push_back({ lines.back().p2, lines.front().p1 });
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
		g.advanceWidth = hmtx->hMetrics[i.glyf_index].advanceWidth;
		g.lsb = hmtx->hMetrics[i.glyf_index].lsb;
        g.rsb = g.advanceWidth - (g.lsb + g.xMax - g.xMin);
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
// issue is overlapping lines and gaps, that creates the line artifacts with the winding rasterization
void readDirectorys(Font_dir* directory, gore::Font* f, FileReader* fr, uint16_t start, uint16_t end) {
	//getting directorys in order we need them
	cmap c_map;
	TTFHeader header;
	std::vector<loca> locas;
	glyph_table g_table;
	table_dir* tab = nullptr;
	fr->resetHead();
	tab = findTable("cmap", directory);
	c_map = readCmap(fr, tab->offset, tab->length, start, end);
	fr->resetHead();
	tab = findTable("head", directory);
	header = readHead(fr, tab->offset, tab->length);
	fr->resetHead();
	tab = findTable("loca", directory);
	locas = readLoca(fr, tab->offset, tab->length, header.indexToLocFormat, &c_map);
	fr->resetHead();
	tab = findTable("hhea", directory);
	hhea_table hhea = readHheaTable(fr, tab->offset, tab->length);
	fr->resetHead();
    tab = findTable("hmtx", directory);
	hmtx_table hmtx = readHmtxTable(fr, tab->offset, tab->length, hhea.numberOfHMetrics, locas.size());
	fr->resetHead();
	tab = findTable("glyf", directory);
	g_table = readGlyfs(fr, tab->offset, tab->length, locas);
	fr->resetHead();
    tab = findTable("vmtx", directory);
    tab = findTable("vhea", directory);
    tab = findTable("GPOS", directory);
	if (tab) {
		readGpos(fr, tab->offset, tab->length, &g_table);
		fr->resetHead();
	}
    tab = findTable("gdef", directory);
    tab = findTable("kern", directory);
	f->unitsPerEm = header.uintsPerEm;
	f->overlap_simple = g_table.overlap_simple;
	constructGlyphs(directory, f, &g_table, &hmtx);
	//don't want to store pointers to anything in gore::Font file
	//https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization, 2.2
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
gore::Font gore::FontLoader::loadFont(std::string file, uint16_t start, uint16_t end) {
	FileReader fr(file);
	//read the gore::Font directory
	Font_dir directory;
	read_offset_subtable(&fr, &directory.off_sub);
	read_table_directory(&fr,  directory.table, directory.off_sub.numTables);
	//now we read all of the directorys we need to
	gore::Font Font;
	Font.name = file;
	Font.overlap_simple = false;
	readDirectorys(&directory, &Font, &fr, start, end);

	return Font;
}
#include "g_engine_2d.h"

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


//ttf file structs

struct off_subtable {
	UINT32 scaler_type; //tag to indicate scaler to be used to razterize font
	UINT16 numTables; //number of tables
	UINT16 searchRange; //(maximum power of 2 <= numTables)*16
	UINT16 entrySelector; //log2(maximum power of 2 <= numTables)
	UINT16 rangeShift; //numTables*16-searchRange
};

struct table_dir {
	std::string t; //human readable tag
	UINT32 tag; //4-byte identifier
	UINT32 checksum; //checksum for the table
	UINT32 offset; //offset from bneging of 'sfnt' (begining of file)
	UINT32 length; //length of table in bytes
};


struct font_dir {
	off_subtable off_sub;
	std::vector<table_dir> table;
};

struct glyph_index {
	int index;
	UINT16 c;
};

struct cmap_table {
	UINT16 platformID;
	UINT16 platformSpecificID;
	UINT32 offset;
	std::vector<glyph_index> indexs;
};



struct cmap {
	UINT16 version;
	UINT16 numTables;
	std::vector<cmap_table> tables;
};


//have to convert these to little endian( I don't know how these macros work but they do so fuck it)
void read_offset_subtable(char* c, off_subtable* table){
	UINT32* t = (UINT32*)c;
	table->scaler_type = SwapFourBytes(*t);
	t++;
	UINT16* te = (UINT16*)t;
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
	UINT32* t = (UINT32*)c;
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

int getOffset(std::string comp, font_dir dir) {
	for (auto& i : dir.table) {
		if (comp.compare(i.t) == 0) {
			return i.offset;
		}
	}
}
struct format4 {
	UINT16 format;
	UINT16 length;
	UINT16 language;
	UINT16 segCountX2;
	UINT16 searchRange;
	UINT16 entrySelector;
	UINT16 rangeShift;
	std::vector<UINT16> endcode;
	UINT16 reservedPad; //left for padding
	std::vector<UINT16> startCode;
	std::vector<UINT16> idDelta;
	std::vector<UINT16> idRangeOffset;
	std::vector<UINT16> glyphIndexArray;
};
int get_glyph_index_format4(UINT16 c, format4* f, UINT16* idRangeStart) {
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
		UINT16* ptr = nullptr;
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
void readFormat4(char* c, cmap_table* table) {
	//now we get to reading, probably make cmap_table have array of glyph indexs so easier to store
	char* m = c + table->offset;
	UINT16* t = (UINT16*)m;
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
	UINT16* idRangeStart = t;
	for (int i = 0; i < form.segCountX2 / 2; i++) {
		form.idRangeOffset.push_back(SwapTwoBytes(*(t + i)));

	}
	t += form.segCountX2 / 2;
	//now we read the glyphidarray
	int remaining = form.length - (((char*)t) - m);

	for (int i = 0; i < remaining / 2; i++) {
		form.glyphIndexArray.push_back(SwapTwoBytes(*(t + i)));
	}
	//now we read all of the character codes
	UINT16 start = 32;
	for (start; start <= 563; start++) {
		table->indexs.push_back({ get_glyph_index_format4(start, &form, idRangeStart), start});
	}
	//std::cout << table->indexs[0].c << " : " << table->indexs[0].index << "\n";
}

//https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
cmap readCmap(char* c, int offset, int length) {
	cmap map;
	char* m = c + offset;
	UINT16* t = (UINT16*)m;
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
		UINT32* te = (UINT32*)t;
		table.offset = SwapFourBytes(*te);
		te++;
		t = (UINT16*)te;
		map.tables.push_back(table); //offset is from start of cmap
		int format = (m + map.tables[i].offset)[0] << 8 | (m + map.tables[i].offset)[1];
		switch (format) {
		case 4:
			//most common
			readFormat4(m, &map.tables[i]);
			break;
		case 0:
			break;
		case 2:
			break;
		case 6:
			break;
		case 8:
			break;
		case 10:
			break;
		case 12:
			//most common
			
			break;
		case 13:
			break;
		case 14:
			break;
		}
	}
	


	return map;
}

//stores character code and offset
struct loca {
	char c;
	UINT32 offset;
};



//read the table when it is the 16 bit version
UINT32 readLoca16(char* s, UINT16 index) {
	UINT16* d = ((UINT16*)s + index);
	return SwapTwoBytes(*d) * 2;
}
//reading the table when it is the 32 bit version
UINT32 readLoca32(char* s, UINT16 index) {
	UINT32* d = (UINT32*)s + index;
	return SwapFourBytes(*d);
}

std::vector<loca> readLoca(char* c, int offset, int length, UINT16 format, cmap* map) {
	char* m = c + offset;
	std::vector<loca> locas;
	if (format == 0) {
		for (int i = 0; i < map->tables[0].indexs.size(); i++) {
			loca l;
			l.c = map->tables[0].indexs[i].c;
			l.offset = readLoca16(m, map->tables[0].indexs[i].index);
			locas.push_back(l);
		}
	}
	else {
		for (int i = 0; i < map->tables[0].indexs.size(); i++) {
			loca l;
			l.c = map->tables[0].indexs[i].c;
			l.offset = readLoca32(m, map->tables[0].indexs[i].index);
			locas.push_back(l);
		}
	}
	return locas;
}

//most of data in here doesn't matter for my uses
struct TTFHeader {
	UINT32 version; //supposed to be fixed but idrc
	UINT32 fontRevision; //set by manufacturer

	UINT32 checkSumAdjustment; //checksum for file; Have to sum entire file as an uint32 and then do 0xB1B0AFBA - sum; 
	UINT32 magicNumber; //idek what this is for supposed to be 0x5F0F3CF5

	UINT16 flags; //every bit gives u flag; These flags can be complex as shit
	UINT16 uintsPerEm; //how may FUnits are in 1 em

	time_t  created; //self explanatory
	time_t modified; //also self explanatory

	short xMin; //supposed to be an FWord but that is just 16 bit signed integer so a short; Also in FUnits
	short yMin;
	short xMax;
	short yMax;

	UINT16 macStyle; //each bit has parameters
	UINT16 lowestRecPPEM; //smallest readable size in pixels
	short fontDirectionHint; //give you hint for directions of glyphs
	short indexToLocFormat; //tells you format of loca table
	short glyphDataFormat; //0 is current format; All i know
};



TTFHeader readHead(char* c, int offset, int length) {
	TTFHeader head;
	char* m = c + offset;
	UINT32* u = (UINT32*)m;
	head.version = SwapFourBytes(*u); //can't swap the bytes on a double for some reason
	u++;
	head.fontRevision = SwapFourBytes(*u); //values will be wrong for the top two since supposed to be fixed
	u++;
	head.checkSumAdjustment = SwapFourBytes(*u);
	u++;
	head.magicNumber = SwapFourBytes(*u);
	u++;
	UINT16* t = (UINT16*)u;
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
	t = (UINT16*)s;
	head.macStyle = *t; //don't swap cause need same bits for flags
	t++;
	head.lowestRecPPEM = SwapTwoBytes(*t);
	t++;
	s = (short*)t;
	head.fontDirectionHint = SwapTwoBytes(*s); 
	s++;
	head.indexToLocFormat = SwapTwoBytes(*s);
	s++;
	head.glyphDataFormat = SwapTwoBytes(*s);
	s++;
	return head;
}



//glyf time

void readGlyf(char* c, int offset, int length) {

}

void readDirectorys(font_dir* directory, Font* f, char* c) {
	//need to sort directorys so you do them in right order
	cmap c_map;
	TTFHeader header;
	std::vector<loca> locas;
	for (auto& i : directory->table) {
		if (i.t.compare("cmap") == 0) {
			c_map = readCmap(c, i.offset, i.length);
		}
		else if (i.t.compare("head") == 0){
			header = readHead(c, i.offset, i.length);
		}
		else if (i.t.compare("loca") == 0) {
			locas = readLoca(c, i.offset, i.length, header.indexToLocFormat, &c_map);
		}
	}
}


//https://docs.fileformat.com/font/ttf/
//https://handmade.network/forums/articles/t/7330-implementing_a_font_reader_and_rasterizer_from_scratch%252C_part_1__ttf_font_reader. part 12
//https://tchayen.github.io/posts/ttf-file-parsing
// https://github.com/RazrFalcon/ttf-parser
//https://learn.microsoft.com/en-us/typography/opentype/spec/ttch01
// http://stevehanov.ca/blog/?id=143
//https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6.html
//big endian so characters will be reversed to me
void FontRenderer::loadFont(std::string file) {
	std::ifstream f;
	f.open(file.c_str(), std::ios::binary);
	//read the file into memory
	std::stringstream stream;
	stream << f.rdbuf();
	std::string st = stream.str();
	char* c = (char*)st.c_str();
	f.close();
	//read the font directory
	font_dir directory;
	read_offset_subtable(c, &directory.off_sub);
	c += 12;
	read_table_directory(c,  directory.table, directory.off_sub.numTables);
	c = (char*)st.c_str(); //reset to begining to get offset easier
	//now we read all of the directorys we need to
	Font font;
	font.name = file;
	readDirectorys(&directory, &font, c);

	return;
}

//draw a bezier curve
void drawBezier() {

}

//https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization#23880
void FontRenderer::drawText(std::string text, Font font) {
	
}
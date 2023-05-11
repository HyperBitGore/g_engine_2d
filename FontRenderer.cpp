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

struct cmap_table {
	UINT16 platformID;
	UINT16 platformSpecificID;
	UINT32 offset;
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




//https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
//need to write support for the formats and figure out how to get format the tables are in
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
	}
	return map;
}

void readDirectorys(font_dir* directory, Font* f, char* c) {
	for (auto& i : directory->table) {
		if (i.t.compare("cmap") == 0) {
			readCmap(c, i.offset, i.length);
		}
	}
}


//https://docs.fileformat.com/font/ttf/
//https://handmade.network/forums/articles/t/7330-implementing_a_font_reader_and_rasterizer_from_scratch%252C_part_1__ttf_font_reader. part 8
//https://tchayen.github.io/posts/ttf-file-parsing
// https://github.com/RazrFalcon/ttf-parser
//https://learn.microsoft.com/en-us/typography/opentype/spec/ttch01
// http://stevehanov.ca/blog/?id=143
//https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6.html
//need to figure out how to turn characters in triangle data
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


void FontRenderer::drawText(std::string text, Font font) {
	
}
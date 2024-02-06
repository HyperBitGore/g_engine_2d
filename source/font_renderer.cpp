#include "font_renderer.hpp"
#include <array>
#include <algorithm>
#include <bitset>



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


int getnthBit(short number, int n) {
	return (number >> n) & 1;
}
int getnthBit(UINT8 number, int n) {
	return (number >> n) & 1;
}


UINT8 swap1Byte(UINT8 n) {
	UINT8 n2 = 0;
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
void readFormat4(char* c, cmap_table* table, UINT16 start, UINT16 end) {
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
	//now we read all of the character codes, change back to 32
	UINT16 start1 = start;
	for (start1; start1 <= end; start1++) {
		table->indexs.push_back({ get_glyph_index_format4(start1, &form, idRangeStart), start1 });
	}
	//std::cout << table->indexs[0].c << " : " << table->indexs[0].index << "\n";
}
//untested
void readFormat0(char* c, cmap_table* table, UINT16 start, UINT16 end) {
	char* m = c + table->offset;
	UINT16* f = (UINT16*)m;
	f++;
	UINT16 length = SwapTwoBytes(*f);
	f++;
	UINT16 language = SwapTwoBytes(*f);
	f++;
	UINT8* g = (UINT8*)f;
	std::vector<UINT8> id_array;
	for (int i = 0; i < length; i++) {
		id_array.push_back(*(g + i));
	}
	UINT16 start1 = start;
	for (start1; start1 <= end; start1++) {
		if (start1 < id_array.size()) {
			table->indexs.push_back({ id_array[start1], start1 });
		}
	}
}
//untested and unfinished
void readFormat2(char* c, cmap_table* table, UINT16 start, UINT16 end) {
	char* m = c + table->offset;
	UINT16* f = (UINT16*)m;
	f++;
	UINT16 length = SwapTwoBytes(*f);
	f+=2; //skipping language 

}
//untested
void readFormat6(char* c, cmap_table* table, UINT16 start, UINT16 end) {
	char* m = c + table->offset;
	UINT16* f = (UINT16*)m;
	UINT16 format = SwapTwoBytes(*f);
	f++;
	UINT16 length = SwapTwoBytes(*f);
	f += 2;//skipping language
	UINT16 firstcode = SwapTwoBytes(*f);
	f++;
	UINT16 entrycount = SwapTwoBytes(*f);
	f++;
	//now we read the glyphidarray
	std::vector<UINT16> glyphidarray;
	for (UINT16 i = 0; i < entrycount; i++) {
		glyphidarray.push_back(*(f + i));
	}

	//outputting to the table
	UINT16 start1 = start;
	for (start1; start1 < end; start1++) {
		int offset = start1 - firstcode;
		if (offset > 0 && offset < entrycount) {
			table->indexs.push_back({ glyphidarray[offset], start1 });
		}
		else {
			table->indexs.push_back({ glyphidarray[0], start1 });
		}
	}
}
void readFormat8(char* c, cmap_table* table, UINT16 start, UINT16 end) {
	char* m = c + table->offset;
	UINT16* f16 = (UINT16*)m;
	f16 += 2;//skipping format and reserved
	UINT32* f32 = (UINT32*)f16;
	UINT32 length = *f32;
	f32 += 2;//skipping language
	//now we read the packed array of bits

}

//untested and im unsure if this is proper way to read this format
void readFormat10(char* c, cmap_table* table, UINT16 start, UINT16 end) {
	char* m = c + table->offset;
	UINT16* f16 = (UINT16*)m;
	f16 += 2; //skipping first two
	UINT32* f32 = (UINT32*)f16;
	UINT32 length = *f32;
	f32+=2;//skipping language
	UINT32 starcharcode = *f32;
	f32++;
	UINT32 numChars = *f32;
	f32++;
	f16 = (UINT16*)f32;
	std::vector<UINT16> glyphindices;
	for (size_t i = 0; i < numChars; i++) {
		glyphindices.push_back(*(f16 + i));
	}
	UINT16 start1 = start;
	for (start1; start1 < end; start1++) {
		int offset = start1 - starcharcode;
		table->indexs.push_back({ glyphindices[offset], start1 });
	}
}
//untested and not done
void readFormat12(char* c, cmap_table* table, UINT16 start, UINT16 end) {
	char* m = c + table->offset;
	UINT16* f = (UINT16*)m;
	f += 2;
	//skipping the format and reserved
	UINT32* t = (UINT32*)f;
	UINT32 length = *t;
	t += 2; //skipping language
	UINT32 numGroups = *t; //number of groupings that follow
	t++;
	std::vector<UINT32> char_codes;
	std::vector<UINT32> indexs;
	size_t seq = 0;
	for (size_t i = 0; i < numGroups; i++) {
		seq = 0;
		UINT32 startcode = *t;
		t++;
		UINT32 endcode = *t;
		t++;
		UINT32 startglyphid = *t;
		t++;
		for (size_t j = startcode; j <= endcode; j++, seq++) {
			char_codes.push_back(*t);
			indexs.push_back((GLuint)startglyphid + (GLuint)seq);
			t++;
		}
	}
	UINT16 start1 = start;
	for (start1; start1 < end; start1++) {
		for (size_t i = 0; i < char_codes.size(); i++) {
			if (char_codes[i] == start1) {
				table->indexs.push_back({ (UINT16)indexs[i], start1});
				break;
			}
		}
		
	}
}

//https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
//https://learn.microsoft.com/en-us/typography/opentype/spec/cmap
cmap readCmap(char* c, int offset, int length, UINT16 start, UINT16 end) {
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
	UINT16 c;
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



struct glyf {
	UINT16 c;
	short numberOfContours;

	//supposed to be FWords but fuck em
	short xMin;
	short yMin;
	short xMax;
	short yMax;
};



struct simp_glyf : glyf {
	UINT16 instructionLength;
	std::vector<UINT8> instructions; 
	std::vector<UINT8> flags;
	std::vector<short> xCoords; //apparently this can also be a uint8 but we'll see
	std::vector<short> yCoords;
	std::vector<UINT16> endPtsOfCountours;
};
//for later use
struct comp_glyf : glyf {

};



struct glyph_table {
	std::vector<simp_glyf> simple_glyphs;
	std::vector<comp_glyf> compound_glyphs;
};



//glyf time

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
			UINT16* t = (UINT16*)s;
			for (int j = 0; j < sg.numberOfContours; j++) {
				sg.endPtsOfCountours.push_back(SwapTwoBytes(*t));
				t++;
			}
			//instructions now, can't believe I forgot to swap this smh, like an hour wasted 
			sg.instructionLength = SwapTwoBytes(*t);
			t++;
			UINT8* d = (UINT8*)t;
			//don't have to swap 
			for (int j = 0; j < sg.instructionLength; j++) {
				sg.instructions.push_back(*d);
				d++;
			}
			//now flags
			int last_index = sg.endPtsOfCountours[sg.numberOfContours - 1];
			for (int j = 0; j < (last_index + 1); j++) {
				sg.flags.push_back(*d);
				d++;
				if (((sg.flags[j] >> 3) & 1) == 1) {
					UINT8 repeat_count = *d;
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
				if (getnthBit(sg.flags[j], 1) == 1) {
					//one byte
					UINT8 temp = *d;
					d++;
					short out = temp;
					if (getnthBit(sg.flags[j], 4) != 1) {
						out *= -1;
					}
					cur_coord = out + prev_coord;
				}
				else {
					//two byte
					if (getnthBit(sg.flags[j], 4) == 1) {
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
				if (getnthBit(sg.flags[j], 2) == 1) {
					//one byte
					UINT8 temp = *d;
					d++;
					short out = temp;
					if (getnthBit(sg.flags[j], 5) != 1) {
						out *= -1;
					}
					cur_coord = out + prev_coord;
				}
				else {
					//two byte
					if (getnthBit(sg.flags[j], 5) == 1) {
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
			//compound glyph do nothing for now
		}

	}
	return table;
}


table_dir* findTable(std::string table, font_dir* directory) {
	for (size_t i = 0; i < directory->table.size(); i++) {
		if (directory->table[i].t.compare(table) == 0) {
			return &directory->table[i];
		}
	}
	return nullptr;
}

void tesslateBezier(std::vector<vec2>& points, vec2 p1, vec2 p2, vec2 p3, int subdiv) {
	float step = 1.0f / subdiv;
	float lx = 0, ly = 0;
	for (int i = 0; i <= subdiv; i++) {
		float t = i * step;
		float t1 = (1.0f - t);
		float t2 = t * t;
		float x = t1 * t1 * p1.x + 2 * t1 * t * p2.x + t2 * p3.x;
		float y = t1 * t1 * p1.y + 2 * t1 * t * p2.y + t2 * p3.y;
		(i == 0) ? lx = x, ly = y : lx, ly;
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

bool compareLine(Line l1, Line l2) {
	return (l1.p1.x == l2.p1.x && l1.p1.y == l2.p1.y && l1.p2.x == l2.p2.x && l1.p2.y == l2.p2.y);
}

void cullEdges(Glyph* g) {
	for (size_t i = 0; i < g->contours.size();) {
		bool cull = false;
		for (size_t j = 0; j < g->contours.size(); j++) {
			if (compareLine(g->contours[i], g->contours[j]) && i != j){ 
				cull = true;
				break;
			}
		}
		if (cull) {
			g->contours.erase(g->contours.begin() + i);
		}
		else {
			i++;
		}
	}
}

void readDirectorys(font_dir* directory, Font* f, char* c, UINT16 start, UINT16 end) {
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
	tab = findTable("glyf", directory);
	g_table = readGlyfs(c, tab->offset, tab->length, locas);
	//don't want to store pointers to anything in font file
	//https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization, 2.2
	for (auto& i : g_table.simple_glyphs) {
		Glyph g;
		g.c = i.c;
		g.xMax = i.xMax;
		g.yMax = i.yMax;
		g.yMin = i.yMin;
		g.xMin = i.xMin;
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
				//g.points.push_back({ (float)i.xCoords[k], (float)i.yCoords[k] });
				if (getnthBit(i.flags[k], 0) == 1) {
					size_t p3_in = k + 1;
					if (k == i.endPtsOfCountours[j]) {
						p3_in = 0;
					}
					vec2 p1 = { (float)i.xCoords[k], (float)i.yCoords[k] };
					vec2 p2 = { (float)i.xCoords[p3_in], (float)i.yCoords[p3_in] };
					vec2 p3;
					p3.x = p2.x + (p1.x - p2.x) / 2.0f;
					p3.y = p2.y + (p1.y - p2.y) / 2.0f;
					//tesslateBezier(&g, p1, p2, p3, 20);
					points.push_back({ x, y});
					//g.points.push_back({ (float)i.xCoords[p3_in], (float)i.yCoords[p3_in] });
					
					
				}
				else{
					//if this is the first contour point
					if (contour_start) {
						contour_started_off = true; 
						//next point is on curve
						if (getnthBit(i.flags[next_index], 0) == 1) {
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
					if (getnthBit(i.flags[next_index], 0) == 1) {
						p3.x = p2.x + (p3.x - p2.x) / 2.0f;
						p3.y = p2.y + (p3.y - p2.y) / 2.0f;
					}
					else {
						k++;
					}
					//g.points.push_back(p1);
					//g.points.push_back(p2);
					//g.points.push_back(p3);
					//generate points
					tesslateBezier(points, p1, p2, p3, 2);
				}
				contour_start = false;
			}
			if (getnthBit(i.flags[k - 1], 0) == 1) {
				//g.points.push_back(g.points[generated_points_start_index]);
				points.push_back({ (float)i.xCoords[contour_start_index] , (float)i.yCoords[contour_start_index] });
			}
			if (contour_started_off) {
				vec2 p1 = points[points.size() - 1];
				vec2 p2;
				p2.x = (float)i.xCoords[contour_start_index];
				p2.y = (float)i.yCoords[contour_start_index];
				vec2 p3 = points[generated_points_start_index];

				//g.points.push_back(p1);
				//g.points.push_back(p2);
				//g.points.push_back(p3);

				tesslateBezier(points, p1, p2, p3, 2);
			}
			end_contours.push_back((int)points.size());
		}
		g.contours = generate_edges(end_contours, points);
		//cull duplicate lines
		cullEdges(&g);
		f->glyphs.push_back(g);
	}

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
Font FontRenderer::loadFont(std::string file, UINT16 start, UINT16 end) {
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
	readDirectorys(&directory, &font, c, start, end);

	return font;
}

bool range(float n, float brange, float trange) {
	return n >= brange && n <= trange;
}


//https://www.youtube.com/watch?v=4bIsntTiKfM
//coding math is the goat
//this can't do collinear lines yet
vec2 getIntersection(Line l1, Line l2) {
	float A1 = (l1.p2.y - l1.p1.y);
	float B1 = (l1.p1.x - l1.p2.x);
	float C1 = (A1 * l1.p1.x + B1 * l1.p1.y);
	float A2 = (l2.p2.y - l2.p1.y);
	float B2 = (l2.p1.x - l2.p2.x);
	float C2 = (A2 * l2.p1.x + B2 * l2.p1.y);
	float denominator = A1 * B2 - A2 * B1;
	
	//checking if line is collinear or parallel
	vec2 delta_l1 = { l1.p2.x - l1.p1.x, l1.p2.y - l1.p1.y };
	vec2 delta_l2 = { l2.p2.x - l2.p1.x, l2.p2.y - l2.p1.y };
	float slope1 = delta_l1.y / delta_l1.x;
	float slope2 = delta_l2.y / delta_l2.x;
	float in1 = l1.p1.y - (slope1 * l1.p1.x);
	float in2 = l2.p1.y - (slope2 * l2.p1.x);

	vec2 inter = { (B2 * C1 - B1 * C2) / denominator, (A1 * C2 - A2 * C1) / denominator };


	if (denominator == 0 && in1 != in2) {
		//line is parallel
		return { -1, -1 };
	}
	else if (in1 == in2) {
		//line is collinear
		return { -2, -2 };
	}
	//checking if intersection lies on the segment
	float rx0 = (inter.x - l1.p1.x) / (l1.p2.x - l1.p1.x);
	float ry0 = (inter.y - l1.p1.y) / (l1.p2.y - l1.p1.y);
	float rx1 = (inter.x - l2.p1.x) / (l2.p2.x - l2.p1.x);
	float ry1 = (inter.y - l2.p1.y) / (l2.p2.y - l2.p1.y);
	if (((rx0 >= 0 && rx0 <= 1) || (ry0 >= 0 && ry0 <= 1)) && ((rx1 >= 0 && rx1 <= 1) || (ry1 >= 0 && ry1 <= 1))) {
		return inter;
	}
	return { -1, -1 };
}

/*float convertToRange(float n, float min, float max, float old_min, float old_max) {
	return ((n - old_min) / (old_max - old_min)) * (max - min) + min;
}*/

RasterGlyph FontRenderer::rasterizeGlyph(Glyph* g, int w, int h, uint32_t color, bool flipx) {
	//have to scale glyph contour points
	std::vector<Line> lines;
	for (size_t i = 0; i < g->contours.size(); i++) {
		Line l = g->contours[i];
		l.p1.x = convertToRange(l.p1.x, 0.0f, (float)w - 1, g->xMin, g->xMax);
		l.p1.y = convertToRange(l.p1.y, 0.0f, (float)h - 1, g->yMin, g->yMax);
		

		l.p2.x = convertToRange(l.p2.x, 0.0f, (float)w - 1, g->xMin, g->xMax);
		l.p2.y = convertToRange(l.p2.y, 0.0f, (float)h - 1, g->yMin, g->yMax);

		lines.push_back(l);
	}
	RasterGlyph r_g;
	std::vector<float> intersections;
	r_g.c = g->c;
	r_g.data = imageloader::createBlank(w, h, 4);
	imageloader::createTexture(r_g.data, GL_RGBA8, GL_RGBA);
	//rewrite this myself cause I think the tutorials version is utter dogshit water, 
	struct sortContours {
		bool operator() (Line l1, Line l2) { return l1.p1.y < l2.p1.y; }
	} sortLines;
	std::sort(lines.begin(), lines.end(), sortLines);

	struct sortInters {
		bool operator() (vec2 l1, vec2 l2) { return l1.y < l2.y; }
	} sortVec2;

	//https://stackoverflow.com/questions/3838329/how-can-i-check-if-two-segments-intersect
	//do vertical scanlines
	for (int x = 0; x < w; x++) {
		Line test_line = { {(float)x, 0.0f}, {(float)x, (float)h} };
		std::vector<vec2> inters; //list of intersection points
		std::vector<Line> adds;
		for (size_t i = 0; i < lines.size(); i++) {
			vec2 l = getIntersection(test_line, lines[i]);
			if (l.x >= 0 && l.x <= w) {
				adds.push_back(lines[i]);
				inters.push_back({ (float)x, (float)l.y});
			}
		}
		std::sort(inters.begin(), inters.end(), sortVec2);
		for (size_t i = 1; i < inters.size() && i < adds.size();) {
			float y1 = inters[i - 1].y;
			float y2 = inters[i].y;
			for (int y = (int)y1; y <= y2; y++) {
				imageloader::setPixel(r_g.data, x, y, color, 4);
			}
			if (inters.size() % 2 == 0) {
				i += 2;
			}
			else {
				i++;
			}
		}
		
	}
	//flip the image
	if (flipx) {
		//hacky way to deal with fucked up L's
		for (int y = 0; y < h - 1; y++) {
			for (int x = 0, x1 = w - 1; x <= x1; x++, x1--) {
				uint32_t c1 = (uint32_t)imageloader::getPixel(r_g.data, x, y, 4);
				uint32_t c2 = (uint32_t)imageloader::getPixel(r_g.data, x1, y, 4);
				imageloader::setPixel(r_g.data, x, y, c2, 4);
				imageloader::setPixel(r_g.data, x1, y, c1, 4);
			}
		}
	}
	else {
		for (int y1 = 0, y2 = h - 1; y1 <= y2; y1++, y2--) {
			//flipping the current rows
			unsigned char* c1 = (unsigned char*)std::malloc(w * 4);
			std::memcpy(c1, r_g.data->data + (y1 * (w * 4)), w * 4);
			unsigned char* c2 = r_g.data->data + (y2 * (w * 4));
			std::memcpy(r_g.data->data + (y1 * (w * 4)), c2, w * 4);
			std::memcpy(c2, c1, w * 4);
			std::free(c1);
		}
	}
	return r_g;
}
//flipx vector will decide what glyphs to flip on x axis instead of the normal y axis
void FontRenderer::rasterizeFont(Font* font, int ptsize, uint32_t color, std::vector<UINT16> flipx) {
	font->ptsize = ptsize;
	for (size_t i = 0; i < font->glyphs.size(); i++) {
		bool flip = false;
		for (auto& j : flipx) {
			if (font->glyphs[i].c == j) {
				flip = true;
				break;
			}
		}
		font->r_glyphs.push_back(rasterizeGlyph(&font->glyphs[i], ptsize, ptsize, color, flip));
		imageloader::updateIMG(font->r_glyphs[font->r_glyphs.size() - 1].data);
		//imageloader::createTexture(font->r_glyphs[font->r_glyphs.size() - 1].data, GL_RGBA8, GL_RGBA);
		//createTexture(font->r_glyphs[font->r_glyphs.size() - 1].data, GL_RGBA8, GL_RGBA);
	}
}

int findFontCharRaster(Font* f, UINT16 c) {
	for (size_t i = 0; i < f->r_glyphs.size(); i++) {
		if (f->r_glyphs[i].c == c) {
			return i;
		}
	}
	return 0;
}
int findFontChar(Font* f, UINT16 c) {
	for (size_t i = 0; i < f->glyphs.size(); i++) {
		if (f->glyphs[i].c == c) {
			return i;
		}
	}
	return 0;
}

void FontRenderer::drawRasterText(Font* font, imagerenderer* img_r, std::string text, float x, float y, int ptsize) {
	if (font->r_glyphs.size() <= 0) {
		std::cout << "Trying to draw an empty raster font " << std::endl;
		return;
	}
	float x1 = x;
	float y1 = y;
	//have to scale images based on ptsize
	float scale = (float)font->ptsize / (float)(font->ptsize / ptsize);
	for (size_t i = 0; i < text.size(); i++) {
		if (text[i] >= 33) {
			int index = findFontCharRaster(font, text[i]);
			img_r->drawImage(font->r_glyphs[index].data, {x1, y1}, {scale, scale});
			//addImageCall( x1, y1, scale, scale);
			//bindImg(font->r_glyphs[index].data);
			//renderImgs(true);
		}
		x1 += scale + 2;
	}
	
}
//https://lspwww.epfl.ch/publications/typography/frsa.pdf
//https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization#23880
//2.4.4
//cutout memory inefficient parts of glyph like points
void FontRenderer::drawText(std::string text, Font* font, float x, float y, int ptsize) {
	float x1 = x;
	float y1 = y;
	for (size_t i = 0; i < text.size(); i++) {
		if (text[i] >= 33) {
			int index = findFontChar(font, text[i]);
			for (size_t j = 0; j < font->glyphs[index].contours.size(); j++) {
				Line l = font->glyphs[index].contours[j];
				//converting line points to ptsize
				l.p1.x = convertToRange(l.p1.x, x1, x1 + ptsize - 1, font->glyphs[index].xMin, font->glyphs[index].xMax);
				l.p1.y = convertToRange(l.p1.y, y1, y1 + ptsize - 1, font->glyphs[index].yMin, font->glyphs[index].yMax);


				l.p2.x = convertToRange(l.p2.x, x1, x1 + ptsize - 1, font->glyphs[index].xMin, font->glyphs[index].xMax);
				l.p2.y = convertToRange(l.p2.y, y1, y1 + ptsize - 1, font->glyphs[index].yMin, font->glyphs[index].yMax);
				pr->addLine(l.p1, l.p2);
			}
		}
		//increase the pos by ptsize and a small gap
		x1 += ptsize + 2;
	}
	pr->drawBufferLine();
}
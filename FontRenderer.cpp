#include "g_engine_2d.h"
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
	//now we read all of the character codes, change back to 32
	UINT16 start = 32;
	for (start; start <= 159; start++) {
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


//glyf time

glyph_table readGlyfs(char* c, int offset, int length, std::vector<loca> locas) {
	glyph_table table;
	for (int i = 0; i < locas.size(); i++) {
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
	for (int i = 0; i < directory->table.size(); i++) {
		if (directory->table[i].t.compare(table) == 0) {
			return &directory->table[i];
		}
	}
	return nullptr;
}

void tesslateBezier(Glyph* g, vec2 p1, vec2 p2, vec2 p3, int subdiv) {
	float step = 1.0 / subdiv;
	float lx = 0, ly = 0;
	for (int i = 0; i <= subdiv; i++) {
		float t = i * step;
		float t1 = (1.0f - t);
		float t2 = t * t;
		float x = t1 * t1 * p1.x + 2 * t1 * t * p2.x + t2 * p3.x;
		float y = t1 * t1 * p1.y + 2 * t1 * t * p2.y + t2 * p3.y;
		(i == 0) ? lx = x, ly = y : lx, ly;
		g->points.push_back({ lx, ly });
		g->points.push_back({ x, y });
		lx = x;
		ly = y;
	}
}


std::vector<Line> generate_edges(Glyph* g) {
	std::vector<Line> lines;
	int j = 0;
	for (int i = 0; i < g->end_contours.size(); i++) {
		int first = j;
		for (; j < g->end_contours[i] - 1; j++) {
			Line l;
			l.p1.x = g->points[j].x;
			l.p1.y = g->points[j].y;
			l.p2.x = g->points[j + 1].x;
			l.p2.y = g->points[j + 1].y;
			lines.push_back(l);
		}
		//have to add endpoint of contour and last point of contour as a line, so we can fix any possible gaps in glyphs
		lines.push_back({ { g->points[first].x, g->points[first].y}, {g->points[j].x, g->points[j].y} }); 
		j++;
	}
	return lines;
}


void readDirectorys(font_dir* directory, Font* f, char* c) {
	//test case for swap1byte
	/*
	UINT8 tf = 10;
	std::bitset<8> x(tf);
	std::cout << x << "\n";
	std::bitset<8> y(swap1Byte(tf));
	std::cout << y << "\n";*/
	//need to sort directorys so you do them in right order
	cmap c_map;
	TTFHeader header;
	std::vector<loca> locas;
	glyph_table g_table;
	table_dir* tab = nullptr;
	tab = findTable("cmap", directory);
	c_map = readCmap(c, tab->offset, tab->length);
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
		g.yMax = i.yMin;
		g.xMin = i.xMin;
		int k = 0;
		for (int j = 0; j < i.numberOfContours; j++) {
			int generated_points_start_index = g.points.size() - 1;
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
					p3.x = p2.x + (p1.x - p2.x) / 2.0;
					p3.y = p2.y + (p1.y - p2.y) / 2.0;
					//tesslateBezier(&g, p1, p2, p3, 20);
					g.points.push_back({ x, y});
					//g.points.push_back({ (float)i.xCoords[p3_in], (float)i.yCoords[p3_in] });
					
					
				}
				else{
					//if this is the first contour point
					if (contour_start) {
						contour_started_off = true; 
						//next point is on curve
						if (getnthBit(i.flags[next_index], 0) == 1) {
							g.points.push_back({ (float)i.xCoords[next_index], (float)i.yCoords[next_index] });
							k++;
							continue;
						}
						x = x + (i.xCoords[next_index] - x) / 2.0;
						y = y + (i.yCoords[next_index] - y) / 2.0;
						g.points.push_back({ x, y });
						
					}
					
					vec2 p1 = g.points[g.points.size() - 1];
					vec2 p2 = { (float)x, (float)y };
					vec2 p3 = { (float)i.xCoords[next_index], (float)i.yCoords[next_index] };
					//get the middle point between p1 and p3
					if (getnthBit(i.flags[next_index], 0) == 1) {
						p3.x = p2.x + (p3.x - p2.x) / 2.0;
						p3.y = p2.y + (p3.y - p2.y) / 2.0;
					}
					else {
						k++;
					}
					//g.points.push_back(p1);
					//g.points.push_back(p2);
					//g.points.push_back(p3);
					//generate points
					tesslateBezier(&g, p1, p2, p3, 5);
				}
				contour_start = false;
			}
			if (getnthBit(i.flags[k - 1], 0) == 1) {
				//g.points.push_back(g.points[generated_points_start_index]);
				g.points.push_back({ (float)i.xCoords[contour_start_index] , (float)i.yCoords[contour_start_index] });
			}
			if (contour_started_off) {
				vec2 p1 = g.points[g.points.size() - 1];
				vec2 p2;
				p2.x = (float)i.xCoords[contour_start_index];
				p2.y = (float)i.yCoords[contour_start_index];
				vec2 p3 = g.points[generated_points_start_index];

				//g.points.push_back(p1);
				//g.points.push_back(p2);
				//g.points.push_back(p3);

				tesslateBezier(&g, p1, p2, p3, 5);
			}
			g.end_contours.push_back(g.points.size());
		}
		g.contours = generate_edges(&g);
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
Font EngineNewGL::loadFont(std::string file) {
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

	return font;
}

void EngineNewGL::rasterizeGlyph(Glyph* g, int w, int h, uint32_t color) {
	//have to scale glyph contour points
	std::vector<Line> lines;
	float scale = (float)h / (float)(g->yMax - g->yMin);
	for (int i = 0; i < g->contours.size(); i++) {
		Line l;
		l.p1.x = (g->contours[i].p1.x - g->xMin) * scale;
		l.p1.y = (g->contours[i].p1.y - g->yMin) * scale;
		l.p2.x = (g->contours[i].p2.x - g->xMin) * scale;
		l.p2.y = (g->contours[i].p2.y - g->yMin) * scale;
		lines.push_back(l);
	}

	std::vector<float> intersections;
	g->data = ImageLoader::generateBlankIMG(w, h);
	//rewrite this myself cause I think the tutorials version is utter dogshit water, 
	//probably just loop through every pixel and check if it would be contained in the contours in which case set that pixel
	for (int i = 0; i < g->data->h; i++) {
		float scanline = i;
		for (int j = 0; j < lines.size(); j++) {
			Line l = lines[j];

			float bigger_y = max(l.p1.y, l.p2.y);
			float smaller_y = min(l.p1.y, l.p2.y);

			//if (scanline <= smaller_y) continue;
			//if (scanline > bigger_y) continue;

			float dx = std::abs(l.p2.x - l.p1.x);
			float dy = std::abs(l.p2.y - l.p1.y);
			//intersections
			if (dy == 0) { continue; }
			float inter;
			if (dx == 0) {
				inter = l.p1.x;
			}
			else {
				inter = (scanline - l.p1.y) * (dx / dy) + l.p1.x;
			}
			intersections.push_back(inter);
		}
		//sort the intersections
		std::sort(intersections.begin(), intersections.end());

		if (intersections.size() > 0) {
			for (int k = 0; k < intersections.size(); k+=2) {
				//filling scanlines
				int start_index = intersections[k];
				int end_index = intersections[k + 1];
				for (int j = start_index; j <= end_index; j++) {
					ImageLoader::setPixel(g->data, j, i, color);
				}
			}
		}
	}
}


void drawChar(UINT16 c, Font font, int ptsize) {

}

//https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization#23880
//2.4.4
//do a bunch of memcpys for when i actually want to draw text
//cutout memory inefficient parts of glyph like points
void EngineNewGL::drawText(std::string text, Font font, int ptsize) {
	for (int i = 0; i < font.glyphs[64].contours.size(); i++) {
		Line l = font.glyphs[64].contours[i];
		//addLinePoints({ l.p1.x / 8 + 250, l.p1.y / 8 + 250 }, { l.p2.x / 8 + 250, l.p2.y / 8 + 250 });
		buffer_2d.push_back({l.p1.x / 8 + 250, l.p1.y / 8 + 250});
		buffer_2d.push_back({ l.p2.x / 8 + 250, l.p2.y / 8 + 250 });
	}
	//drawPoints();
	drawLines(0.1f);
}
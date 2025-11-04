#include "font_renderer.hpp"
#include "font_loader.hpp"
#include <fstream>
#include <sstream>

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

struct ChainedSequenceRule {
	std::vector<uint16_t> backtrackSequence;
	std::vector<uint16_t> inputSequence;
	std::vector<uint16_t> lookaheadSequence;
	std::vector<SequenceLookup> seqLookupRecords;
};

// https://learn.microsoft.com/en-us/typography/opentype/spec/gpos
void readGpos (char* c, int32_t offset, int32_t length) {
	char* t = c + offset;
	gpos_1_1 gposheader = *(gpos_1_1*)(t);
	gposheader.majorVersion = SwapTwoBytes(gposheader.majorVersion);
	gposheader.minorVersion = SwapTwoBytes(gposheader.minorVersion);
	gposheader.scriptListOffset = SwapTwoBytes(gposheader.scriptListOffset);
	gposheader.featureListOffset = SwapTwoBytes(gposheader.featureListOffset);
	gposheader.lookupListOffset = SwapTwoBytes(gposheader.lookupListOffset);
	gposheader.featureVariationsOffset = SwapFourBytes(gposheader.featureVariationsOffset);

	// parsing feature list
	char* featurelist = t + gposheader.featureListOffset;
	uint16_t featureCount = SwapTwoBytes(*(uint16_t*)(featurelist));
	std::vector<FeatureRecord> feature_records;
	for (size_t i = 0; i < featureCount; i++) {
		FeatureRecord fr = *(FeatureRecord*)(featurelist + 2 + (i * sizeof(FeatureRecord)));
		fr.featureTag = SwapFourBytes(fr.featureTag);
		fr.featureOffset = SwapTwoBytes(fr.featureOffset);
		feature_records.push_back(fr);
	}
	// parsing lookup list
	char* lookuplist = t + gposheader.lookupListOffset;
	uint16_t lookup_count = SwapTwoBytes(*(uint16_t*)(lookuplist));
	std::vector<ChainedSequenceRule> chainedSeqRules;
	for (size_t i = 0; i < lookup_count; i++) {
		uint16_t look = SwapTwoBytes(*(uint16_t*)(lookuplist + 2 + (i * sizeof(uint16_t))));
		char* off = lookuplist + look;
		LookupTable lt = *(LookupTable*)(off);
		lt.lookupFlag = SwapTwoBytes(lt.lookupFlag);
		lt.lookupType = SwapTwoBytes(lt.lookupType);
		lt.subTableCount = SwapTwoBytes(lt.subTableCount);
		uint32_t lookupTableOff = sizeof(LookupTable);
		if (lt.lookupType == GPOS_POSITIONING_EXTENSION) {
			char* cur_off = off + sizeof(LookupTable);
			uint16_t format = SwapTwoBytes(*(uint16_t*)(cur_off));
			uint16_t lookup_type = SwapTwoBytes(*(uint16_t*)(cur_off + 2));
			uint32_t extension_offset = SwapTwoBytes(*(uint32_t*)(cur_off + 4));
			lt.lookupType = lookup_type;
			lookupTableOff += extension_offset;
		}
		switch (lt.lookupType) {
			case GPOS_SINGLE_ADJUSTMENT:
			break;
			case GPOS_PAIR_ADJUSTMENT:
			break;
			case GPOS_CURSIVE_ATTACHMENT:
			break;
			case GPOS_MARK_TO_BASE_ATTACHMENT:
			break;
			case GPOS_MARK_TO_LIGATURE_ATTACHMENT:
			break;
			case GPOS_MARK_TO_MARK_ATTACHMENT:
			break;
			case GPOS_CONTEXTUAL_POSITIONING:
			break;
			case GPOS_CHAINED_CONTEXTS_POSITIONING:
				{
					char* cur_off = off + lookupTableOff;
					uint16_t format = SwapTwoBytes(*(uint16_t*)(cur_off));
					uint16_t coverageOffset = SwapTwoBytes(*(uint16_t*)(cur_off + 2));
					uint16_t chainedSeqRuleCount = SwapTwoBytes(*(uint16_t*)(cur_off + 4));
					// process the ChainedSequenceRuleSet table
					for (size_t j = 0; j < chainedSeqRuleCount; j++) {
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
					}
				}
			break;
		}
	}
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
// issue is overlapping lines and gaps, that creates the line artifacts with the winding rasterization
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
    tab = findTable("GPOS", directory);
	if (tab) {
		readGpos(c, tab->offset, tab->length);
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
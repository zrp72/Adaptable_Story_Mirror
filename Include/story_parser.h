#ifndef story_parser_header
#define story_parser_header 1
#include <charconv>
#include <cctype>
#include <string>
#include <unordered_map>

#include "utils.h"
#include "glyphtext.h"

enum LCTN_T{
	PRG,
	NXS
};

struct data_change {
	struct string dg;
	struct string var;
	int op;
	int val;
};
using sv_changes = std::vector<data_change>;
using data_group = std::unordered_map<struct string, int, string_hash, string_equal>;
using story_vars = std::unordered_map<struct string, data_group, string_hash, string_equal>;

struct txtr_info {
	char* border;

	//TODO: Implement animations
	char* entry_anim;
	char* exit_anim;
};

struct styling {
	txtr_info paragraph;
	txtr_info option;
	glyph_styling glyph;
	std::unordered_map<struct string, struct string, string_hash> markers;
};

struct option {
	vector<glyph> prompt;
	char* dst_label;
	story_vars reveal_conds;
	story_vars changes;
	txtr_info txtr_data;
};

struct paragraph {
	LCTN_T t;

	vector<glyph> text;
	story_vars changes;
	vector<option> options;
	txtr_info txtr_data;
};

struct nexus {
	LCTN_T t;

	struct exit_group {
		story_vars conds;
		vector<struct string> exits;
	};
	
	vector<exit_group> exits;
	int visits;
};

struct story {
	std::unordered_map<struct string, void*, string_hash> tag_map;
	vector<glyph> used_glyphs;
	char* location;
	story_vars data_groups;
	std::unordered_map<struct string, struct string, string_hash> dg_aliases;
};

struct story* parse_story_file(char* filepath);

#endif


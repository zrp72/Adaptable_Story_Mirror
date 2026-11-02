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
	struct zstring dg;
	struct zstring var;
	int op;
	int val;
};
using sv_changes = std::vector<data_change>;
using data_group = std::unordered_map<struct zstring, int, string_hash, string_equal>;
using story_vars = std::unordered_map<struct zstring, data_group, string_hash, string_equal>;

struct txtr_info {
	//This will be changed to struct string once other files are updated
	std::string border;

	//TODO: Implement animations
	char* entry_anim;
	char* exit_anim;
};

struct styling {
	txtr_info paragraph;
	txtr_info option;
	glyph_styling glyph;
	std::unordered_map<struct zstring, struct zstring, string_hash> markers;
};

struct option {
	vector<glyph> prompt;
	struct zstring dst_label;
	sv_changes reveal_conds;
	sv_changes changes;
	txtr_info txtr_data;
};

struct paragraph {
	LCTN_T t;

	vector<glyph> text;
	sv_changes changes;
	vector<struct option*> options;
	txtr_info txtr_data;
};

struct exit_group {
	sv_changes conds;
	vector<struct zstring> exits;
};

struct nexus {
	LCTN_T t;

	vector<exit_group> exits;
	int visits;
};

struct story {
	std::unordered_map<struct zstring, void*, string_hash> tag_map;
	vector<glyph> used_glyphs;
	struct zstring location;
	story_vars data_groups;
	std::unordered_map<struct zstring, struct zstring, string_hash> dg_aliases;
};

struct story* parse_story_file(const char* filepath, char* start_chapter);
void free_story(struct story*);

#endif


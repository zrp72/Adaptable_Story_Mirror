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

typedef data_group = std::unordered_map<struct string, int, cstring_hash>;
typedef story_vars = std::unordered_map<struct string, data_group, string_hash>;

struct txtr_info {
	char* border;
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
	struct LCTN_T t;

	vector<glyph> text;
	story_vars changes;
	vector<option> options;
	txtr_info texture_data;
};

struct nexus {
	struct LCTN_T t;

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


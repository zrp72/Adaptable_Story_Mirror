#include <charconv>
#include <cctype>
#include <string>
#include <unordered_map>

#include "utils.h"
#include "glyphtext.h"


typedef data_group = umap<char*, int>;
typedef story_vars = umap<char*, data_group>;

struct {
	char* border;
	char* entry_anim;
	char* exit_anim;
} txtr_info;

struct {
	txtr_info paragraph;
	txtr_info option;

	umap<char*, bool> bold_markers;
	umap<char*, bool> italic_markers;
} styling;

struct {
	vector<glyph> prompt;
	char* dst_label;
	story_vars reveal_conds;
	story_vars changes;
	txtr_info txtr_data;
} option;

struct {
	vector<glyph> text;
	story_vars changes;
	vector<option> options;
	txtr_info texture_data;
} paragraph;

struct {
	struct {
		story_vars conditions;
		vector<std::string> exits;
	} exit_group;
	
	vector<exit_group> exits;
	int visits;
} nexus;

struct {
	umap<char*, std::pair<char, int>> tag_map;
	vector<paragraph> paragraphs;
	vector<nexus> nexuses;
	vector<glyph> used_glyphs;
	char* story_location;
} story;

struct {
	story story;
	umap<char*, item> items;
	character player;
} game_data;

struct story* parse_story_file(char* filepath);

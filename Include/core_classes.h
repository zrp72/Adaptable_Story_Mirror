#ifndef classes_header
#define classes_header 1
#include <iostream>
#include <map>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <functional>
#include <list>

#include "basic_functions.h"
#include "glyphtext.h"

class item {
	public:
	std::string ID;
	std::string item_type;
	std::map<std::string, int> stat_bonus;

	item(): ID(""){}
	item(std::string i) : ID(i){}
};

class item_container {
	public:
	std::unordered_map<std::string, int> items;
	size_t max_size = 0;
	size_t used_space = 0;

	item_container(){}

	bool contains(item* i){
		return items.contains(i->ID);
	}

	bool contains(std::string i){
		return items.contains(i);
	}
	
	int& at(std::string s){
		return items.at(s);
	}
};

class character {
	public:
	std::map<std::string, int> attributes;
	std::string lineage;
	item_container inventory;
	std::map<std::string, bool> decisions;

	character(){}
};

class texture_info {
	public:
	std::string border;
	std::string entry_anim;
	std::string exit_anim;

	texture_info(){};
	texture_info(std::string b, std::string en, std::string ex)
	: border(b), entry_anim(en), exit_anim(ex){}
};

class styling {
	public:
	texture_info paragraph;
	texture_info option;

	std::unordered_map<std::string, bool> bold_markers;
	std::unordered_map<std::string, bool> italic_markers;

	styling(){};

	void add_marker(std::string, std::pair<char, bool>);
	void remove_marker(std::string);
	
};

class story_variables {
	public:
	std::vector<std::pair<std::string, int>> stats_and_items;
	std::vector<std::string> decisions;

	story_variables(){}

	void clear(){
		this->stats_and_items.clear();
		this->decisions.clear();
	};

	bool empty(){
		if(!stats_and_items.empty()){return false;}
		if(!decisions.empty()){return false;}
		return true;
	}
};

class option {
	public:
	std::vector<glyph> prompt;
	std::string dest_label;
	story_variables reveal_conditions;
	story_variables changes;
	texture_info texture_data;

	option(std::string dest, texture_info t)
	: dest_label(dest), texture_data(t){}
};

class paragraph{
	public:
	std::vector<glyph> text;
	story_variables changes;
	std::vector<option> options;
	texture_info texture_data;

	paragraph(texture_info t): texture_data(t){}
};

class nexus {
	public:

	class exit_group {
		public:
		story_variables conditions;
		std::vector<std::string> exits;

		exit_group(){}
	};

	std::vector<exit_group> exits;
	int visits = 0;

	nexus(){}
};

class story {
	public:
	std::map<std::string, std::pair<char, int>> tag_map;
	std::vector<paragraph> paragraphs;
	std::vector<nexus> nexuses;
	std::vector<glyph> used_glyphs;
	std::string story_location;

	story(){}
	void parse_story_file(std::filesystem::path);
};

class game_data {
	public:
	story story;
	std::map<std::string, item> items;
	character player;

	game_data(std::string l){
		story.story_location = l;
	}
};

class gui_core {
	public:
	SDL_Window* window;
	SDL_Renderer* renderer;
	TTF_TextEngine* text_engine;

	gui_core(){
		if(!SDL_Init(SDL_INIT_VIDEO)){
			throw std::runtime_error(SDL_GetError());
		}

		if(!TTF_Init()){
			throw std::runtime_error(SDL_GetError());
		}

		window = SDL_CreateWindow("TextRPG", 800, 600, SDL_WINDOW_RESIZABLE);
		if(!window){
			throw std::runtime_error(SDL_GetError());
		}

		renderer = SDL_CreateRenderer(window, NULL);
		if(!renderer){
			throw std::runtime_error(SDL_GetError());
		}

		text_engine = TTF_CreateRendererTextEngine(renderer);
		if(!text_engine){
			throw std::runtime_error(SDL_GetError());
		}
	}

	~gui_core(){
		TTF_DestroyRendererTextEngine(text_engine);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
	}
};

class assets;

class relative_rect {
	public:
	SDL_Rect SDL_rect;
	//x position relative to screen size, from left to right, 0... 1.
	float relative_x = 0;
	//y position relative to screen size, from top to bottom, 0... 1.
	float relative_y = 0;
	//width relative to screen size, 0... 1.
	float relative_w = 1;
	//height relative to screen size, 0... 1.
	float relative_h = 1;

	relative_rect(){}
	relative_rect(float x, float y, float w)
	: relative_x(x), relative_y(y), relative_w(w){
		relative_h = 0;
	}
	relative_rect(float x, float y, float w, float h)
	: relative_x(x), relative_y(y), relative_w(w), relative_h(h){}
};

class relative_text {
	public:
	glyphtext text;
	relative_rect pos;
	relative_rect mask;
	int scroll_y = 0;

	relative_text(){};
	relative_text(glyphtext t, relative_rect m, relative_rect p)
	: text(t), mask(m), pos(p){}
};

class relative_texture {
	public:
	std::string texture_ID;
	SDL_Rect srcrect;
	relative_rect dstrect;
	std::function<void(game_data&, assets&, gui_core&)> click_events = nullptr;

	relative_texture(){};
	relative_texture(std::string t, SDL_Rect s, relative_rect d, std::function<void(game_data&, assets&, gui_core&)> c)
	: texture_ID(t), srcrect(s), dstrect(d), click_events(c){};
};

class UI_element {
	public:
	std::vector<relative_texture> texture;
	std::vector<relative_text> text;
	relative_rect mask;
	relative_rect area;
	int scroll_y = 0;
	bool visible = false;

	UI_element(){}

	void determine_area();
};

class assets {
	public:
	//Includes glyph atlas textures
	std::unordered_map<std::string, SDL_Texture*> textures;
	std::unordered_map<glyph, rasterized_glyph, glyph_hash> glyphs;
	std::map<std::string, UI_element> elements;

	assets(gui_core&);

	~assets(){
		for(auto& [tag, t] : textures){
			SDL_DestroyTexture(t);
		}
	}

	void add_element(std::string, UI_element&, gui_core&);
	SDL_Rect get_texture_srcrect(std::string);
};

#endif
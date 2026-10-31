#ifndef gui_header
#define gui_header 1

#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>

#include "story_parser.h"

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
	std::function<void(struct story&, assets&, gui_core&)> click_events = nullptr;

	relative_texture(){};
	relative_texture(std::string t, SDL_Rect s, relative_rect d, std::function<void(struct story&, assets&, gui_core&)> c)
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

void resize_rect(relative_rect&, SDL_Window*);
void resize_element(UI_element&, gui_core&, assets&);
void render_debug_hitbox(gui_core&, SDL_Rect, int, int, int, int);
void render_element(UI_element&, gui_core&, assets&);
int create_story_display(gui_core&, assets&, struct story&);
int create_option_display(gui_core&, assets&, struct story&);
int create_UI_elements(gui_core&, assets&, struct story&);

#endif


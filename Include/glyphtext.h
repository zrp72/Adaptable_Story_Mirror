#ifndef glyphtext_header
#define glyphtext_header 1

#include <iostream>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#define GLYPHTEXT_ALIGN_CENTER 0
#define GLYPHTEXT_ALIGN_LEFT 1
#define GLYPHTEXT_ALIGN_RIGHT 2
#define GLYPHTEXT_ALIGN_JUSTIFY 3


class glyph_styling {
	public:
	//FONT SHOULD BE THE FILENAME(can exclude the .ttf)
	std::string font = "roboto_light";
	SDL_Color color = {255, 255, 255, 255};
	bool bold = false;
	bool italic = false;
	int size = 24;

	glyph_styling(){}
};

class glyph {
	public:
	uint32_t unicode;
	glyph_styling style;

	glyph(){}
	glyph(uint32_t u, glyph_styling& s)
		: unicode(u), style(s){}
	glyph(char u, glyph_styling& s)
		: unicode(u), style(s){}

	bool operator==(const glyph&) const;

	bool operator<(const glyph&) const;
};

class glyph_hash {
	public:
	size_t operator()(const glyph&) const;
};

class rasterized_glyph {
	public:
	std::string atlas_tag;
	SDL_FRect position_on_atlas;
	uint32_t unicode;
	std::unordered_map<size_t, int> kerning;
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int advance;
	int descent;
	int ascent;

	rasterized_glyph(){}
};

class glyphtext {
	public:
	//Word and line are mainly used for refreshing layout
	class word {
		public:
		std::vector<glyph> glyphs;
		int length = 0;

		word(){}
		void clear();
	};
	class line {
		public:
		std::vector<word> words;
		int length = 0;
		
		line(){}
		void clear();
	};

	std::vector<glyph> glyphs;
	//Words is the whitespace terminated version of glyphs
	//used for layout so that words aren't split between lines
	std::vector<word> words;
	int ascent = 0;
	int descent = 0;
	int default_space = 0;
	std::vector<SDL_FRect> layout;
	int height = 0;

	glyphtext(){}
	glyphtext(std::vector<glyph> g):glyphs(g){}
	void refresh_words(std::unordered_map<glyph, rasterized_glyph, glyph_hash>&);
	void refresh_layout(std::unordered_map<glyph, rasterized_glyph, glyph_hash>&, int, int);
};

class packing_grid {
	public:
	size_t width = 0;
	size_t height = 0;
	std::vector<std::vector<bool>> pixels;

	//Takes width and height to create a vector<vector<bool>>(a grid of pixels either available or used)
	packing_grid(size_t, size_t);

	//Returns an xy pixel location for where the object can fit from its top left corner.
	//Padding should always be an even number.
	//If no position is found, the returned point will be -1, -1
	std::pair<int, int> find_space(int, int, int);

	//Takes the x, y, width, and height of the space being used.
	//Will return false if given area goes beyond pixel grid bounds.
	//Will print a debug if it attempts to use a pixel already used.
	bool use_pixels(int, int, int, int);
};

std::unordered_map<glyph, rasterized_glyph, glyph_hash>
rasterize_glyph_atlas(std::filesystem::path, std::vector<glyph>&, 
	SDL_Renderer*, std::unordered_map<std::string, SDL_Texture*>&, std::string, int, int);

#endif

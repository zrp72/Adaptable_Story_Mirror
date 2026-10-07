#include "glyphtext.h"
#define debug(x) std::cout << x

bool glyph::operator==(const glyph& other) const{
	if(unicode != other.unicode){return false;}
	if(style.font != other.style.font){return false;}
	if(style.color.r != other.style.color.r){return false;}
	if(style.color.g != other.style.color.g){return false;}
	if(style.color.b != other.style.color.b){return false;}
	if(style.color.a != other.style.color.a){return false;}
	if(style.bold != other.style.bold){return false;}
	if(style.italic != other.style.italic){return false;}
	if(style.size != other.style.size){return false;}
	return true;
}

bool glyph::operator<(const glyph& other) const{
	if(unicode < other.unicode){return true;}
	if(style.color.r < other.style.color.r){return true;}
	if(style.color.g < other.style.color.g){return true;}
	if(style.color.b < other.style.color.b){return true;}
	if(style.color.a < other.style.color.a){return true;}
	if(style.bold != other.style.bold){return true;}
	if(style.italic != other.style.italic){return true;}
	if(style.size < other.style.size){return true;}
	return false;
}

size_t glyph_hash::operator()(const glyph& glyph) const{
	//TODO: Learn how to write a good hash function
	size_t u = glyph.unicode;
	size_t r = glyph.style.color.r << 1;
	size_t g = glyph.style.color.g * 5;
	size_t b = glyph.style.color.b / 30;
	size_t a = glyph.style.color.a + 44;
	size_t bi = glyph.style.bold + glyph.style.italic * 2;
	size_t s = glyph.style.size >> 1;
	return u + r + g + b + a + bi + s;
}

void glyphtext::word::clear(){
	glyphs.clear();
	length = 0;
}

void glyphtext::line::clear(){
	words.clear();
	length = 0;
}

//Takes width and height to create a vector<vector<bool>>(a grid of pixels either available or used)
packing_grid::packing_grid(size_t w, size_t h):width(w), height(h){
	for(int x = 0; x < width; ++x){
		std::vector<bool> column;
		for(int y = 0; y < height; ++y){
			column.push_back(false);
		}
		pixels.push_back(column);
	}
}

//Returns an xy pixel location for where the object can fit from its top left corner.
//Padding should always be an even number.
//If no position is found, the returned point will be -1, -1
std::pair<int, int> packing_grid::find_space(int w, int h, int padding){
	for(int x = 0; x < width; ++x){
		for(int y = 0; y < height; ++y){
			if(pixels.at(x).at(y)){
				continue;
			}else{
				bool viable = true;
				for(int px = 0; px < w + padding && x + px < width; ++px){
					if(x + px >= width){
						viable = false;
						break;
					}
					for(int py = 0; py < h + padding; ++py){
						if(pixels.at(x + px).at(y + py)){
							viable = false;
							break;
						}
						if(y + py >= height){
							viable = false;
							break;
						}
					}
					if(!viable){
						break;
					}
				}
				if(viable){
					return {x + (padding / 2), y + (padding / 2)};
				}
			}
		}
	}
	return {-1, -1};
}

//Takes the x, y, width, and height of the space being used.
//Will return false if given area goes beyond pixel grid bounds.
//Will print a debug if it attempts to use a pixel already used.
bool packing_grid::use_pixels(int x, int y, int w, int h){
	for(int px = 0; px < w; ++px){
		if(x + px >= width){return false;}
		for(int py = 0; py < h; ++py){
			if(y + py >= height){return false;}
			pixels.at(x + px).at(y + py) = true;
		}
	}
	return true;
}

//The passed directory is where all ttf font files should reside.
//This function assumes that glyph font names will match their filenames.
//
//Takes a vector of every glyph to rasterize and a map to store atlas textures.
//
//Atlas name defines what the rasterized glyphs will store their atlas tags as.
//Given atlas name will be followed by the number related to its iteration, starting at 1.
//
//Return an unordered map of glyphs to their rasterized positions and data.
std::unordered_map<glyph, rasterized_glyph, glyph_hash>
rasterize_glyph_atlas(std::filesystem::path directory, std::vector<glyph>& glyphs, 
	SDL_Renderer* renderer, std::unordered_map<std::string, SDL_Texture*>& container, 
	std::string atlas_name, int atlas_w, int atlas_h){

	std::filesystem::path original_directory = SDL_GetBasePath();
	if(!std::filesystem::exists(directory)){
		debug("Passed directory does not exist: " << directory << '\n');
		return {};
	}
	std::filesystem::current_path(directory);
	debug("Moved to directory: " << std::filesystem::current_path() << '\n');

	std::unordered_map<glyph, rasterized_glyph, glyph_hash> rasterized_glyphs;
	std::unordered_map<std::string, TTF_Font*> cached_fonts;

	int atlas_iteration = 1;
	std::string atlas_tag = atlas_name + std::to_string(atlas_iteration);

	packing_grid packing_container(atlas_w, atlas_h);
	SDL_Surface* atlas = SDL_CreateSurface(atlas_w, atlas_h, SDL_PIXELFORMAT_RGBA8888);
	for(auto& glyph : glyphs){

		if(!cached_fonts.contains(glyph.style.font)){
			std::filesystem::path path(glyph.style.font);
			if(!path.has_extension()){
				path += ".ttf";
			}else if(path.extension() != ".ttf"){
				debug("Attempted to rasterize glyph with unknown font file type(not .ttf): " << path << '\n');
				return rasterized_glyphs;
			}
			
			if(!std::filesystem::exists(path)){
				debug("Cannot find font file \"" << glyph.style.font << "\".\n");
				continue;
			}

			TTF_Font* font = TTF_OpenFont(path.string().data(), 24);
			TTF_SetFontKerning(font, true);
			cached_fonts.emplace(glyph.style.font, font);
		}

		TTF_Font* font = cached_fonts.at(glyph.style.font);
		if(glyph.style.bold){TTF_SetFontStyle(font, TTF_STYLE_BOLD);}
		if(glyph.style.italic){TTF_SetFontStyle(font, TTF_STYLE_ITALIC);}
		if(glyph.style.bold && glyph.style.italic){TTF_SetFontStyle(font, TTF_STYLE_BOLD | TTF_STYLE_ITALIC);}
		TTF_SetFontSize(font, glyph.style.size);
		if(!TTF_FontHasGlyph(font, glyph.unicode)){
			debug("Font \"" << glyph.style.font << "\" does not contain glyph: " << char(glyph.unicode) << '\n');
			continue;
		}
		SDL_Surface* g = TTF_RenderGlyph_Blended(font, glyph.unicode, glyph.style.color);
		TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

		if(g == nullptr){
			debug("Failed to load glyph surface.\n");
			continue;
		}

		rasterized_glyph rg;
		TTF_GetGlyphMetrics(font, glyph.unicode,
			&rg.min_x,
			&rg.max_x,
			&rg.min_y,
			&rg.max_y,
			&rg.advance
		);
		rg.descent = TTF_GetFontDescent(font);
		rg.ascent = TTF_GetFontAscent(font);
		if(TTF_GetFontKerning(font)){
			for(auto& g : glyphs){
				if(!rg.kerning.contains(g.unicode) && TTF_FontHasGlyph(font, g.unicode)){
					int kerning;
					TTF_GetGlyphKerning(font, g.unicode, glyph.unicode, &kerning);
					if(kerning != 0){
						rg.kerning.emplace(g.unicode, kerning);
					}
				}
			}
		}else{
			debug("Kerning is disabled for font " << glyph.style.font << '\n');
		}

		auto dst = packing_container.find_space(g->w, g->h, 2);
		if(dst.first == -1){

			SDL_Texture* a = SDL_CreateTextureFromSurface(renderer, atlas);
			container.emplace(atlas_tag, a);
			SDL_ClearSurface(atlas, 0, 0, 0, 0);
			atlas_iteration += 1;
			atlas_tag = atlas_name + std::to_string(atlas_iteration);

			packing_container = packing_grid(atlas_w, atlas_h);
			dst = packing_container.find_space(g->w, g->h, 2);
			if(dst.first == -1){
				debug("Glyph is larger than glyph atlas bounds.");
				continue;
			}
		}
		if(!packing_container.use_pixels(dst.first, dst.second, g->w, g->h)){
			debug("Attempted to pack glyph outside pixel bounds.");
		}
		SDL_Rect dstrect(dst.first, dst.second, g->w, g->h);
		SDL_BlitSurface(g, NULL, atlas, &dstrect);

		SDL_RectToFRect(&dstrect, &rg.position_on_atlas);
		rg.atlas_tag = atlas_tag;
		rg.unicode = glyph.unicode;

		rasterized_glyphs.emplace(glyph, rg);
	}
	SDL_Texture* a = SDL_CreateTextureFromSurface(renderer, atlas);
	container.emplace(atlas_tag, a);
	SDL_DestroySurface(atlas);

	for(std::pair<std::string, TTF_Font*> font : cached_fonts){
		TTF_CloseFont(font.second);
	}
	std::filesystem::current_path(original_directory);
	return rasterized_glyphs;
}

//Seperates core glyphs by whitespace to create a vector of vectors of glyphs mainly used for layout purposes.
//
//This function is automatically called by refresh layout if the amount of cached words is not equal to the amount of words in the core_glyphs. 
//Automatic calling does not happen if core_glyphs are added without whitespace seperation.
void
glyphtext::refresh_words(std::unordered_map<glyph, rasterized_glyph, glyph_hash>& ras_info){
//TODO: Make sure every type of whitespace is handled
	glyphtext::word word;
	size_t previous;
	for(auto& g : this->glyphs){
		ascent = std::max(ascent, ras_info.at(g).ascent);
		descent = std::min(descent, ras_info.at(g).descent);

		auto& g_info = ras_info.at(g);

		switch(g.unicode){
		case ' ':
			default_space = std::max(default_space, g_info.advance);
			if(!word.glyphs.empty()){
				words.push_back(word);
				word.clear();
			}
			break;

		case '\n':
			word.glyphs.push_back(g);
			words.push_back(word);
			word.clear();
			break;

		default:
			word.glyphs.push_back(g);

			if(g_info.kerning.contains(previous)){
				word.length += g_info.kerning.at(previous);
			}
			
			word.length += g_info.advance;
		}

		previous = g.unicode;
	}
	if(!word.glyphs.empty()){
		words.push_back(word);
		word.clear();
	}
}

//Lays out glyphs with SDL_FRects's starting at their top-left corners.
//First point may not always be 0, 0
//Points are NOT given for whitespace(or atleast shouldnt be).
//
//Alignments are:
//GLYPHTEXT_ALIGN_CENTER
//GLYPHTEXT_ALIGN_LEFT
//GLYPHTEXT_ALIGN_RIGHT
//GLYPHTEXT_ALIGN_JUSTIFY
void
glyphtext::refresh_layout(std::unordered_map<glyph, rasterized_glyph, glyph_hash>& ras_info, int w, int alignment){
	layout.clear();

	int word_count = 0;
	bool previous_space = true;
	for(auto& glyph : glyphs){
		if(!isspace(glyph.unicode)){
			if(previous_space){
				word_count += 1;
				previous_space = false;
			}
		}else{
			previous_space = true;
		}
	}
	if(word_count != this->words.size()){
		refresh_words(ras_info);
	}

	std::vector<glyphtext::line> lines;
	glyphtext::line line;

	auto next_line = [&](){
		lines.push_back(line);
		line.clear();
	};

	for(auto& word : words){
		int spaces = line.words.size() - 1;
		if(alignment == GLYPHTEXT_ALIGN_JUSTIFY){
			if(spaces != -1 && (w - (line.length + word.length)) / (spaces + 1) < default_space){
				next_line();
			}
		}else if(line.length + (default_space * (spaces + 1)) + word.length > w){
			next_line();
		}

		if(word.length > w){
			glyphtext::word split_word;
			size_t previous;
			for(auto& glyph : word.glyphs){
				auto& g_info = ras_info.at(glyph);

				if(glyph.unicode == '\n'){
					line.words.push_back(split_word);
					line.length += split_word.length;
					split_word.clear();
					next_line();
					continue;
				}

				if(g_info.kerning.contains(previous)){
					line.length += g_info.kerning.at(previous);
				}

				if(split_word.length + g_info.advance > w){
					line.words.push_back(split_word);
					line.length += split_word.length;
					split_word.clear();
					next_line();
				}

				split_word.glyphs.push_back(glyph);
				split_word.length += g_info.advance;
				previous = glyph.unicode;
			}

			line.words.push_back(split_word);
			line.length += split_word.length;
			continue;
		}

		if(word.glyphs.back().unicode == '\n'){
			word.glyphs.pop_back();
			line.words.push_back(word);
			next_line();
		}else{
			line.words.push_back(word);
			line.length += word.length;
		}

	}
	if(!line.words.empty()){
		lines.push_back(line);
		line.clear();
	}
	
	int font_height = ascent - descent;
	int pen_y = font_height + descent;
	int pen_x = 0;
	double line_padding = 1.4;
	int lowest_point = 0;
	size_t previous = ' ';

	auto append_layout = [&](glyphtext::word& word){
		for(auto& glyph : word.glyphs){
			auto& rg = ras_info.at(glyph);
			
			if(rg.kerning.contains(previous)){
				debug("Glyph " << char(rg.unicode) << " kerning for " << char(previous) << " is " << rg.kerning.at(previous) << '\n');
				pen_x += rg.kerning.at(previous);
			}

			layout.push_back(
				SDL_FRect(pen_x + rg.min_x, 
				pen_y - rg.ascent, 
				rg.position_on_atlas.w, 
				rg.position_on_atlas.h)
			);

			lowest_point = std::max(lowest_point, pen_y - rg.descent);
			pen_x += rg.advance;
			previous = rg.unicode;
		}
		previous = ' ';
	};

	auto layout_line = [&](glyphtext::line& l, int& end_space){
		for(auto& word : l.words){
			append_layout(word);
			pen_x += end_space;
		}
	};

	for(int line_it = 0; line_it < lines.size(); ++line_it){
		auto& line = lines.at(line_it);
		int spaces = line.words.size() - 1;
		bool last_line = line_it == lines.size() - 1;

		if(alignment == GLYPHTEXT_ALIGN_LEFT){

			layout_line(line, default_space);

		}else if(alignment == GLYPHTEXT_ALIGN_RIGHT){

			pen_x = w - (line.length + (spaces * default_space));
			layout_line(line, default_space);

		}else if(alignment == GLYPHTEXT_ALIGN_CENTER){

			pen_x = (w - (line.length + (spaces * default_space))) / 2;
			layout_line(line, default_space);

		}else if(alignment == GLYPHTEXT_ALIGN_JUSTIFY){

			if(spaces == 0){
				layout_line(line, default_space);
			}else if(last_line){
				pen_x = (w - (line.length + (spaces * default_space))) / 2;
				layout_line(line, default_space);
			}else{
				int spacing = (w - line.length) / spaces;
				layout_line(line, spacing);
			}
		}
		pen_y += font_height * line_padding;
		pen_x = 0;
	}

	this->height = lowest_point;
}
#include "utils.h"
#include "gui.h"
#include "progress_story.h"


void
UI_element::determine_area(){
	float min_x = 1;
	float min_y = 1;
	float top_w = 0;
	float top_h = 0;

	auto resize = [&](float x, float y, float w, float h){
		min_x = std::min(min_x, x);
		min_y = std::min(min_y, y);
		top_w = std::max(top_w, x + w);
		top_h = std::max(top_h, y + h);
	};

	for(auto &tex : this->texture){
		auto t = tex.dstrect;
		resize(t.relative_x, t.relative_y, t.relative_w, t.relative_h);
	}
	for(auto &txt : this->text){
		auto& c = txt.pos;
		auto& m = txt.mask;
		resize(c.relative_x, c.relative_y, c.relative_w, m.relative_h);
	}

	this->area = {min_x, min_y, top_w - min_x, top_h - min_y};
}

assets::assets(gui_core& gui, char* rel_path){

	std::filesystem::path original_path(std::filesystem::current_path());
	std::filesystem::current_path(original_path / rel_path);

	auto load_texture = [&](std::string texture_name, const char* path){
		SDL_Texture* texture = IMG_LoadTexture(gui.renderer, path);
		if(!texture){
			std::cerr << "Failed to load texture: " << SDL_GetError() << '\n';
		}
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		this->textures[texture_name] = texture;
	};

	//TODO: Load all asset files in rel_path
	load_texture("option_border", "option_border.png");

	std::filesystem::current_path(original_path);
}

void
assets::add_element(std::string tag, UI_element& e, gui_core& gui){
	e.determine_area();
	elements.insert_or_assign(tag, e);
}

SDL_Rect
assets::get_texture_srcrect(std::string tag){
	float width, height;
	SDL_GetTextureSize(textures[tag], &width, &height);
	SDL_Rect srcrect(0, 0, width, height);
	return srcrect;
}

void
resize_rect(relative_rect &rect, SDL_Window* window){
	int window_w;
	int window_h;
	SDL_GetWindowSizeInPixels(window, &window_w, &window_h);

	rect.SDL_rect.x = window_w * rect.relative_x;
	rect.SDL_rect.y = window_h * rect.relative_y;

	rect.SDL_rect.w = window_w * rect.relative_w;
	if(rect.relative_h != 0){
		rect.SDL_rect.h = window_h * rect.relative_h;
	}else{
		rect.SDL_rect.h = 0;
	}
}

void
resize_element(UI_element &obj, gui_core& gui, assets& assets){
	resize_rect(obj.area, gui.window);
	resize_rect(obj.mask, gui.window);
	for (auto &texture : obj.texture) {
		resize_rect(texture.dstrect, gui.window);
	}
	for (auto &text : obj.text) {
		resize_rect(text.mask, gui.window);
		resize_rect(text.pos, gui.window);
		text.text.refresh_layout(assets.glyphs, text.pos.SDL_rect.w, 
			GLYPHTEXT_ALIGN_JUSTIFY);

		int max_scroll = text.text.height - text.mask.SDL_rect.h;
		max_scroll = std::max(0, max_scroll);
		text.scroll_y = std::clamp(text.scroll_y, 0, max_scroll);
	}

	int max_scroll = obj.area.SDL_rect.h - obj.mask.SDL_rect.h;
	max_scroll = std::max(0, max_scroll);
	obj.scroll_y = std::clamp(obj.scroll_y, 0, max_scroll);
}

void
render_debug_hitbox(gui_core& gui, SDL_Rect area, int r, int g, int b, int a){
	SDL_SetRenderDrawColor(gui.renderer, r, g, b, a);
	SDL_FRect frect1;
	SDL_RectToFRect(&area, &frect1);
	SDL_RenderRect(gui.renderer, &frect1);
}

void
render_element(UI_element& obj, gui_core& gui, assets& assets){

	render_debug_hitbox(gui, obj.area.SDL_rect, 255, 192, 203, 255);

	SDL_SetRenderClipRect(gui.renderer, &obj.mask.SDL_rect);
	SDL_SetRenderTarget(gui.renderer, NULL);

	for(auto &t : obj.texture){
		SDL_FRect srcrect;
		SDL_FRect dstrect;
		SDL_RectToFRect(&t.srcrect, &srcrect);
		SDL_RectToFRect(&t.dstrect.SDL_rect, &dstrect);
		dstrect.y -= obj.scroll_y;

		SDL_Texture* texture = assets.textures[t.texture_ID];
		SDL_RenderTexture(gui.renderer, texture, &srcrect, &dstrect);
	}

	for(auto &t : obj.text){

		SDL_Rect text_mask = t.mask.SDL_rect;
		text_mask.y -= obj.scroll_y;

		SDL_Rect mask;
		if(SDL_GetRectIntersection(&text_mask, &obj.mask.SDL_rect, &mask)){
			SDL_SetRenderClipRect(gui.renderer, &mask);

			render_debug_hitbox(gui, mask, 255, 255, 255, 255);

			int l = 0;
			for(auto& glyph : t.text.glyphs){
				if(!std::isspace(glyph.unicode)){
					if(l >= t.text.layout.size()){
						debug("Layout does not contain positions for remaining glyphs.\n");
						break;
					}else{
						SDL_FRect dst = t.text.layout.at(l);
						dst.x += t.pos.SDL_rect.x;
						dst.y += t.pos.SDL_rect.y - t.scroll_y;
						auto& atlas = assets.textures.at(assets.glyphs.at(glyph).atlas_tag);
						auto& src = assets.glyphs.at(glyph).position_on_atlas;
						SDL_RenderTexture(gui.renderer, atlas, &src, &dst);
						l += 1;
					}
				}
			}
		}
	}
	SDL_SetRenderClipRect(gui.renderer, NULL);
}

int
create_story_display(gui_core& gui, assets& assets, struct story& story){
	UI_element element;
	struct paragraph* location = (struct paragraph*)story.tag_map[story.location];
	if(location->t != PRG){
		fprintf(stderr, "Location set to nexus \"%s\"\n", story.location.text);
		exit(1);
	}
	auto& story_text = location->text;
	auto& border = location->txtr_data.border;
	if(assets.textures.contains(border)){
		fprintf(stderr, "Attempted to use unloaded texture: %s\n", border.c_str());
		exit(1);
	}

	element.text.push_back(relative_text(
		story_text,
		relative_rect(.15, .1, .7, .5),
		relative_rect(.15, .1, .7)
	));

	/* TODO: Implement paragraph borders
	element.texture.push_back(relative_texture(
		border,
		assets.get_texture_srcrect(border),
		relative_rect(x_pos -.05, y_pos, .4, .05),
		[=](struct story& data, assets& assets, gui_core& gui){}
	));
	*/

	for(auto& g : element.text[0].text.glyphs){
		debug(char(g.unicode));
	}
	debug('\n');

	element.visible = true;
	assets.add_element("story_display", element, gui);
	return 0;
}

int
create_option_display(gui_core& gui, assets& main_assets, struct story& story){
	auto& data = story.data_groups;

	UI_element element;

	int column = 0;
	int row = 0;
	float column_w = .3;
	float column_h = .05;

	struct paragraph* location = (struct paragraph*)story.tag_map[story.location];
	if(location->t == NXS){
		fprintf(stderr, "Story location set to nexus \"%s\"\n", story.location.text);
		exit(1);
	}

	int iteration = 1;
	for(auto& option : location->options){
		debug("Option iteration: " << iteration << '\n');
		if(column > 1){row += 1; column = 0;}

		auto& rvl_cnds = option->reveal_conds;
		bool viable = true;
		for(auto& dc : rvl_cnds){
			if(!data.contains(dc.dg)){
				fprintf(stderr, "Attempted to compare against nonexistant data group \"%s\"\n", dc.dg.text);
			} else if( !data[dc.dg].contains(dc.var) ){
				continue;
			}
			if( false == (viable = compare_sv(dc.val, dc.op, data[dc.dg][dc.var])) ){
				goto end;
			}
		}

		auto& border = option->txtr_data.border;
		if(main_assets.textures.contains(border)){
			fprintf(stderr, "Attempted to use unloaded texture: %s\n", border.c_str());
			exit(1);
		}

		for(auto& t : option->prompt){
			debug(char(t.unicode));
		}
		debug('\n');

		float x_pos = .15 + .4 * column;
		float y_pos = .7 + .1 * row;
		relative_rect mask(x_pos, y_pos, column_w, column_h);

		auto& prompt = option->prompt;
		element.text.push_back(relative_text(
			prompt,
			mask,
			relative_rect(x_pos, y_pos, .3)
		));

		element.texture.push_back(relative_texture(
			border,
			main_assets.get_texture_srcrect(border),
			relative_rect(x_pos -.05, y_pos, .4, .05),
			[=](struct story& data, assets& assets, gui_core& gui){
				progress_story(data, assets, gui, iteration);
			}
		));

		column += 1;
end:
		iteration += 1;
	}

	element.mask = relative_rect(.1, .7, .85, .3);
	element.visible = true;

	debug("Amount of options: " << element.text.size() << '\n');

	main_assets.add_element("option_display", element, gui);

	return 0;
}

int
create_UI_elements(gui_core& gui, assets& assets, struct story& story){

	create_story_display(gui, assets, story);
	create_option_display(gui, assets, story);

	for(auto& element : assets.elements){
		resize_element(element.second, gui, assets);
	}

	return 0;
}

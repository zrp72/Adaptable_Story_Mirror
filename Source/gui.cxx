#include "gui.h"
#include "utils.h"

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


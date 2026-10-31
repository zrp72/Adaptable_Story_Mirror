#include "launch.h"

int
launch_game(struct story& data, assets& assets, gui_core& gui){

	bool running = true;
	float mouse_x;
	float mouse_y;
	SDL_Point mouse_pos;

	while(running){
		SDL_Event event;

		while(SDL_PollEvent(&event)){
			if(event.type == SDL_EVENT_QUIT){
				running = false;
			}

			if(event.type == SDL_EVENT_WINDOW_RESIZED){
				for(auto &[tag, obj] : assets.elements){
					resize_element(obj, gui, assets);
				}
			}

			if(event.type == SDL_EVENT_MOUSE_MOTION){
				SDL_GetMouseState(&mouse_x, &mouse_y);
				mouse_pos = SDL_Point(mouse_x, mouse_y);
			}

			if(event.type == SDL_EVENT_MOUSE_WHEEL){

				auto clamp_scroll = [](int &max_scroll, int &scroll_y){
					max_scroll = std::max(0, max_scroll);
					scroll_y = std::clamp(scroll_y, 0, max_scroll);
				};

				bool scrolled = false;
				for(auto &[tag, obj] : assets.elements){

					for(auto &text : obj.text){

						if(SDL_PointInRect(&mouse_pos, &text.mask.SDL_rect)){
							int max_scroll = text.text.height - text.mask.SDL_rect.h;
							text.scroll_y -= event.wheel.y * 25;
							clamp_scroll(max_scroll, text.scroll_y);

							if(max_scroll != 0){scrolled = true;}
							break;
						}
					}

					if(scrolled){break;}else{

						if(SDL_PointInRect(&mouse_pos, &obj.area.SDL_rect)){
							int max_scroll = obj.area.SDL_rect.h - obj.mask.SDL_rect.h;
							obj.scroll_y -= event.wheel.y * 25;
							clamp_scroll(max_scroll, obj.scroll_y);
							break;
						}
					}
				}
			}

			if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
				bool clicked = false;

				for(auto& [tag, obj] : assets.elements){
					for(auto& t : obj.texture){

						SDL_Rect visible_dstrect(t.dstrect.SDL_rect);
						visible_dstrect.y -= obj.scroll_y;

						if(SDL_PointInRect(&mouse_pos, &visible_dstrect)){
							if(t.click_events){
								t.click_events(data, assets, gui);
								clicked = true;
								break;
							}
						}
					}
					if(clicked){break;}
				}
			}
		}

		SDL_SetRenderDrawColor(gui.renderer, 0, 0, 0, 255);
		SDL_SetRenderTarget(gui.renderer, NULL);
		SDL_RenderClear(gui.renderer);

		for (auto &[tag, obj] : assets.elements){
			if(obj.visible){
				render_element(obj, gui, assets);
			}
		}
		
		SDL_RenderTexture(gui.renderer, assets.textures.at("glyph_atlas1"), NULL, NULL);

		SDL_RenderPresent(gui.renderer);
	}

	return 0;
}

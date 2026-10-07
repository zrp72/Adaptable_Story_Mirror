#include <filesystem>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include "core_classes.h"

#define debug(x)  std::cout << x

assets::assets(gui_core& gui){

	std::filesystem::path original_path(std::filesystem::current_path());
	std::filesystem::current_path(original_path / "assets");

	auto load_texture = [&](std::string texture_name, const char* path){
		SDL_Texture* texture = IMG_LoadTexture(gui.renderer, path);
		if(!texture){
			std::cerr << "Failed to load texture: " << SDL_GetError() << '\n';
		}
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		this->textures[texture_name] = texture;
	};

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
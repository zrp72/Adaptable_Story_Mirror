#include <iostream>
#include <filesystem>
#include <sdl3/SDL_filesystem.h>

#include "instruction_loader.h"
#include "functions.h"
#include "glyphtext.h"

#define debug(x)  std::cout << x

int main(int argc, char** argv) {
	try {
		std::filesystem::current_path(SDL_GetBasePath());
		
		struct RAL config;
		char* config_file = argc > 1 ? argv[1] : "READ_DIRECTIONS";
		load_ral(config_file, &config);

		game_data data(config.start_chapter);
		data.story.parse_story_file(std::filesystem::path("Story.txt"));

		gui_core gui;
		assets assets(gui);
		assets.glyphs = rasterize_glyph_atlas(std::filesystem::path("ttf_files"), 
			data.story.used_glyphs, gui.renderer, assets.textures, "glyph_atlas", 1024, 1024);

		if(create_UI_elements(gui, assets, data)){
			debug("Failed to create UI elements");
			return 1;
		}

		if(launch_game(data, assets, gui)){
			debug("Failed to launch game");
			return 1;
		}

	}catch (const std::exception &a){
		return 1;
	}
	return 0;
}

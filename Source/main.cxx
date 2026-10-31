#include <iostream>
#include <filesystem>
#include <sdl3/SDL_filesystem.h>

#include "utils.h"
#include "instruction_loader.h"
#include "story_parser.h"
#include "gui.h"
#include "glyphtext.h"
#include "launch.h"

int main(int argc, char** argv) {
	try {
		std::filesystem::current_path(SDL_GetBasePath());
		
		struct RAL config;
		char* config_file = argc > 1 ? argv[1] : "READ_DIRECTIONS";
		load_ral(config_file, &config);

		struct story* story = parse_story_file(config.story_file, config.start_chapter);

		gui_core gui;
		assets assets(gui, config.asset_folder);
		assets.glyphs = rasterize_glyph_atlas(std::filesystem::path("ttf_files"), 
			story.used_glyphs, gui.renderer, assets.textures, "glyph_atlas", 1024, 1024);

		if(create_UI_elements(gui, assets, story)){
			debug("Failed to create UI elements");
			return 1;
		}

		if(launch_game(data, assets, gui)){
			debug("Failed to launch game");
			return 1;
		}

		free_story(story);

	}catch (const std::exception &a){
		return 1;
	}
	return 0;
}

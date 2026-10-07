#include <iostream>
#include <filesystem>

#include "functions.h"

#define debug(x) std::cout << x

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

int
create_story_display(gui_core& gui, assets& assets, game_data& data){
	UI_element element;
	auto& location = data.story.tag_map[data.story.story_location];
	auto& story_text = data.story.paragraphs[location.second].text;

	element.text.push_back(relative_text(
		story_text,
		relative_rect(.15, .1, .7, .5),
		relative_rect(.15, .1, .7)
	));
	for(auto& g : element.text[0].text.glyphs){
		debug(char(g.unicode));
	}
	debug('\n');

	element.visible = true;
	assets.add_element("story_display", element, gui);
	return 0;
}

int
create_option_display(gui_core& gui, assets& main_assets, game_data& data){
	UI_element element;

	int column = 0;
	int row = 0;
	float column_w = .3;
	float column_h = .05;

	std::pair<char, int> location = data.story.tag_map[data.story.story_location];
	if(location.first == 'n'){
		debug("Story location set to nexus? " << data.story.story_location << '\n');
		return 1;
	}

	int iteration = 1;
	for(auto& option : data.story.paragraphs[location.second].options){
		debug("Option iteration: " << iteration << '\n');
		if(column > 1){row += 1; column = 0;}
		if(!option.reveal_conditions.empty()){
			if(!compare_variables_against_player(data, option.reveal_conditions)){
				iteration += 1;
				continue;
			}
		}

		for(auto& t : option.prompt){
			debug(char(t.unicode));
		}
		debug('\n');

		float x_pos = .15 + .4 * column;
		float y_pos = .7 + .1 * row;
		relative_rect mask(x_pos, y_pos, column_w, column_h);

		auto& prompt = option.prompt;
		element.text.push_back(relative_text(
			prompt,
			mask,
			relative_rect(x_pos, y_pos, .3)
		));

		element.texture.push_back(relative_texture(
			"option_border",
			main_assets.get_texture_srcrect("option_border"),
			relative_rect(x_pos -.05, y_pos, .4, .05),
			[=](game_data& data, assets& assets, gui_core& gui){
				progress_story(data, assets, gui, iteration);
			}
		));

		column += 1;
		iteration += 1;
	}

	element.mask = relative_rect(.1, .7, .85, .3);
	element.visible = true;

	debug("Amount of options: " << element.text.size() << '\n');

	main_assets.add_element("option_display", element, gui);

	return 0;
}

int
create_UI_elements(gui_core& gui, assets& assets, game_data& data){

	create_story_display(gui, assets, data);
	create_option_display(gui, assets, data);

	for(auto& element : assets.elements){
		resize_element(element.second, gui, assets);
	}

	return 0;
}
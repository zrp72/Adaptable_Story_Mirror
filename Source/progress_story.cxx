#include <iostream>
#include <vector>
#include <map>

#include <stdexcept> //may not be needed
#include <cerrno>

#include "functions.h"
#include "basic_functions.h"

#define debug(x)  std::cout << x

bool
compare_variables_against_player(game_data& data, story_variables& variables){
	auto& items = data.items;
	auto& attributes = data.player.attributes;
	auto& decisions = data.player.decisions;
	auto& inv = data.player.inventory;

	bool viable = true;
	for(auto& condition : variables.stats_and_items){
		if(items.contains(condition.first)){
			if(!inv.contains(condition.first)){
				viable = false;
			}
		} else {
			if(attributes.contains(condition.first)){
				viable = attributes.at(condition.first) >= condition.second;
			} else {
				debug("Assuming unassigned attribute: " << condition.first << '\n');
				viable = false;
			}
		}
		if(!viable){break;}
	}
	
	if(viable){
		for(auto& decision : variables.decisions){
			if(!decisions.contains(decision)){
				viable = false;
				break;
			}
		}
	}
	return viable;
}

std::string*
get_destination(game_data &data, nexus &nexus){

	nexus.visits += 1;

	std::vector<std::pair<char, int>> destination;
	int highest_exit_group = -1;

	for(auto& eg : nexus.exits){

		bool viable = true;
		auto it = eg.conditions.stats_and_items.begin();

		while(it != eg.conditions.stats_and_items.end()){
			if(it->first == "visits" || it->first == "visit"){
				if(nexus.visits < it->second){
					viable = false;
				}
				eg.conditions.stats_and_items.erase(it);
				break;
			}
			++it;
		}

		if(!viable){continue;}

		if(!compare_variables_against_player(data, eg.conditions)){
			continue;
		}

		highest_exit_group += 1;
	}
	if(highest_exit_group == -1){
		debug("No exit found for nexus.\n");
	}

	auto& exits = nexus.exits.at(highest_exit_group).exits;
	if(exits.size() == 0){
		debug("Nexus exit group has no exits.\n");
	}
	int exit = randomize_number(exits.size() - 1);
	return &exits.at(exit);
}

//TODO: Move this function into class item_container
void
alter_inventory(character& player, auto& change){
	auto& inventory = player.inventory;
	int open_inventory_space = player.inventory.max_size - inventory.used_space;

	//TODO: Account for stacking
	if((open_inventory_space - change.second) < 0){
		debug("Overburdened" << '\n');
	}

	if(inventory.contains(change.first)){
		inventory.at(change.first) += change.second;
		debug(change.first << " + " << change.second << '\n');
	}else{
		inventory.items.emplace(change.first, change.second);
		debug("New item: " << change.first << '\n');
	}
}

//handles option and paragraph change maps, so auto is NEEDED
void
apply_changes(game_data &data, story_variables& changes){
	for (const auto& change : changes.stats_and_items) {
		if(data.items.contains(change.first)){
			alter_inventory(data.player, change);
		}else{
			if(!data.player.attributes.contains(change.first)){

				debug("New attribute created: " << change.first << '\n');
				data.player.attributes[change.first] = change.second;

			}else{

				debug(change.first << ": " << data.player.attributes.at(change.first) << '\n');
				data.player.attributes.at(change.first) += change.second;

			}
			debug(change.first << ": " << data.player.attributes.at(change.first) << '\n');
		}
	}

	for (const auto& change : changes.decisions) {

		data.player.decisions.emplace(change, true);
		debug("Added decision: " << change << '\n');

	}
}

int
update_UI(game_data& data, assets& assets, gui_core& gui){

	create_story_display(gui, assets, data);
	resize_element(assets.elements.at("story_display"), gui, assets);

	create_option_display(gui, assets, data);
	resize_element(assets.elements.at("option_display"), gui, assets);

	return 0;
}

int
progress_story(game_data &data, assets& main_assets, gui_core& gui, int choice){
	auto& tag_map = data.story.tag_map;
	auto& location = data.story.tag_map.at(data.story.story_location);
	paragraph& p = data.story.paragraphs.at(location.second);

	if(choice > p.options.size()){
		debug("Clicked option does not exist.\n");
		return 1;
	} else {

		option& c = p.options[choice - 1];

		if(!c.changes.empty()){
			apply_changes(data, c.changes);
		}

		debug("Dest label: " << c.dest_label << '\n');
		std::string* dest = &c.dest_label;

		while(tag_map.at(*dest).first == 'n'){
			auto& nexus = data.story.nexuses.at(tag_map.at(*dest).second);
			dest = get_destination(data, nexus);
		}

		data.story.story_location = *dest;
		paragraph& new_location = data.story.paragraphs.at(tag_map.at(*dest).second);

		if(!new_location.changes.empty()){
			apply_changes(data, new_location.changes);
		}

		update_UI(data, main_assets, gui);
		//save_game(data);
	}
	return 0;
}
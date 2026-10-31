#include <iostream>
#include <vector>
#include <map>

#include <stdexcept> //may not be needed
#include <cerrno>

#include "functions.h"
#include "basic_functions.h"

#define debug(x)  std::cout << x

//Returns true if sv1 has all the variables of sv2 at the same or higher values
bool
compare_sv(story_vars& sv1, story_vars& sv2){

	bool viable = true;
	for(const auto &[dgn, dg] : sv1){
		if(sv2.contains(dgn){
			for(const auto& [var, val] : dg){
				if(sv2[dgn].contains(var)){
					viable = val >= sv2[dgn][var];
				}
			}
		}
		if(!viable){break;}
	}
	return viable;
}

struct string
get_destination(story &story, nexus &nexus){

	nexus.visits += 1;

	std::vector<std::pair<char, int>> destination;
	int highest_exit_group = -1;

	for(auto& eg : nexus.exits){

		bool viable = true;
		if(eg.conds.contains(reserved)){
			if(eg.conds[reserved].contains(visits)){
				viable = eg.conds[reserved][visits] > nexus.visits;
			}
		}
		if(!viable){continue;}

		if(!compare_sv(eg.conds, story.data_groups)){
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

void
apply_changes(struct story &story, sv_changes& changes){
	for(const auto& c : changes){
		if(!story.data_groups.contains(c.dg)){
			fprintf(stderr, "Attempted to alter nonexistent data group: %s", dgn.text);
			exit(1);
		}
		if(!story.data_groups[c.dg].contains(c.var)){
			story.data_groups[c.dg][c.var] = 0;
		}
		switch(c.op){
		case '=':
			story.data_groups[c.dg][c.var] = c.val;
			continue;
		case '+':
			story.data_groups[c.dg][c.var] + c.val;
			continue;
		case '-':
			story.data_groups[c.dg][c.var] - c.val;
			continue;
		}
	}
}

int
update_UI(struct story& data, assets& assets, gui_core& gui){

	create_story_display(gui, assets, data);
	resize_element(assets.elements.at("story_display"), gui, assets);

	create_option_display(gui, assets, data);
	resize_element(assets.elements.at("option_display"), gui, assets);

	return 0;
}

int
progress_story(struct story &data, assets& main_assets, gui_core& gui, int choice){
	auto& tag_map = story.tag_map;
	void* location = tag_map.at(story.location);
	if(location->t == NXS){
		fprintf(stderr, "Story location set to nexus(%s)?\n", story.location);
		exit(1);
	}

	if(choice > location.options.size()){
		debug("Clicked option does not exist.\n");
		exit(1);
	} else {

		option& c = location.options[choice - 1];

		if(!c.changes.empty()){
			apply_changes(data, c.changes);
		}

		debug("Dest label: " << c.dst_label.text << '\n');
		void* dst = tag_map[c.dst_label];
		struct string* dst = &c.dst_label;

		while(tag_map[*dst]->t == NXS){
			*dst = get_destination(story, *tag_map[*dst]);
		}

		story.location = *dst;
		paragraph& new_location = tag_map[*dst];

		if(!new_location.changes.empty()){
			apply_changes(story, new_location.changes);
		}

		update_UI(story, main_assets, gui);
		//save_game(data);
	}
	return 0;
}

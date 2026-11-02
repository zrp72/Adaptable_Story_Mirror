#include <iostream>
#include <vector>
#include <map>

#include <stdexcept> //may not be needed
#include <cerrno>

#include "story_parser.h"
#include "utils.h"

//Returns true if sv1 has all the variables of sv2 at the same or higher values
bool
compare_sv(int val1, int op, int val2){
	switch(op){
	case '>':
		return val1 > val2;
	case '<':
		return val1 < val2;
	case '=':
		return val1 = val2;
	}
	fprintf(stderr, "Invalid comparison operator \"%c\"\n", op);
	exit(1);
}

struct zstring
get_destination(story &story, nexus &nexus){

	auto& data = story.data_groups;

	nexus.visits += 1;

	std::vector<std::pair<char, int>> destination;
	int highest_exit_group = -1;

	int current_exit_group = -1;
	for(auto& eg : nexus.exits){
		current_exit_group += 1;

		bool viable = true;
		for(auto& dc : eg.conds){
			if( 0 == strncmp(dc.dg.text, "reserved", 8) ){
				if( 0 == strncmp(dc.var.text, "visit", 5) ){
					viable = compare_sv(dc.val, dc.op, nexus.visits);
				} else {
					if(!data.contains(dc.dg)){
						fprintf(stderr, "Attempted to compare against nonexistant data group \"%s\"\n", dc.dg.text);
					} else if( !data[dc.dg].contains(var) ){
						continue;
					}
					viable = compare_sv(dc.val, dc.op, data[dc.dg][dc.var]);
				}
			}
			if(!viable){
				break;
			}
		}
		if(!viable){continue;}

		highest_exit_group = current_exit_group;
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
		default:
			fprintf(stderr, "Invalid change operator on change \"<%s>%s%c%d\"\n", c.dg.text, c.var.text, c.op, c.val);
			exit(1)
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
		struct zstring* dst = &c.dst_label;

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

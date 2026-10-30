#include "story_parser.h"
#include "utils.h"

//NEW CODE

//File move past char
//Returns false on EOF
bool
fmove_pc(FILE* file, char target){
	char c;
	while(EOF != (c = fgetc(file))){
		if(c == target){
			return true;
		}
	}
	return false;
}

//File move past whitespace
//Returns -1 on EOF
//Otherwise returns the first non-ws char
int
fmove_pw(FILE* f){
	int c;
	while(EOF != (c = fgetc(f))){
		if(!isspace(c)){
			fungetc(c, f);
			return c;
		}
	}
	return false;
}

//Match key iteratively
int
match_key(
	char c, 
	char* key, 
	int pos
){
	if(c == key[pos]){
		if(key[pos+1] == '\0'){
			return 1;
		}
		return 0;

	} else if(isspace(c) && (key[pos-1] == '\n')){
		//NOTE: All whitespace after a newline WILL be trimmed.
		return 0;

	} else if(ki > 0){
		return -1;
	}
	return -2;
}

void
app_failed_key(struct string* s, char* key, int ki);
	for(int i = 0; i < ki; i++){
		dynamic_append(key[i], s);
	}
}

int
list_contains_c(char* list, char c){
	for(int i = 0; list[i] != '\0'; i++){
		if(c == list[i]){
			return 1;
		}
	}
	return 0;
}

//Get file substring to single char
//Char* end is treated as a list of
//single chars.
//NOTES:
//
//|Returns EOF if end on EOF,
//	otherwise returns the last char read
//
//|Places the file iterator
//	past the end character
//
//|Trims whitespace if string is
//	terminated on \n
int
gfss_tsc(FILE* f, struct string* string, char* end){
	int c;
	while(EOF != (c = fgetc(f))){
		if(list_contains_c(end, c)){
			break;
		}
		dynamic_append(c, string);
	}
	if(c == '\n'){
		trim_tw(string);
	} else if(c == EOF) {
		return EOF;
	}
	return c;
}

//Get file substring to multi char
//Char* end is treated as a key
//NOTES:
//Returns EOF if end on EOF
//Returns 1 if the key was found
//Places the file iterator
//	past the end character
//Trims trailing whitespace 
//	if string is the last 
//	char read is \n
int
gfss_tmc(FILE* f, struct string* string, char* end){	

	int ki = 0;
	int c;
	while(EOF != (c = fgetc(f))){
		int s = match_key(c, end, ki);
		switch(s){
		case -2://todo: is this even needed?
			break;
		case -1:
			app_failed_key(string, end, ki);
			ki = 0;
			break;
		case 0:
			ki += 1;
			continue;
		case 1:
			goto finished;
		}
		
		dynamic_append(c, string);
	}
finished:
	if(c == '\n'){
		trim_tw(string);
	} else if(c == EOF) {
		return EOF;
	}
	return 1;
}

//TODO: Factor
int
parse_sv(FILE* f, story_vars* sv, char* ends){
	int rv;
	struct string cdg = get_str(8);
	while(rv = fmove_pw(f)){
		if(list_contains_c(ends, rv)){
			break;
		}

		if(rv == '<'){
			gfss_tsc(f, &cdg, ">)");
			if(!*sv.contains(cdg)){
				data_group* dg = new *dg();
				*sv[cdg] = dg;
			}
		} else if(cdg.len == 0) {
			printf("Active data group not declared during sv parse.");
			return -1;
		} else if(rv == ','){
			fseek(f, 1, SEEK_CUR);
		} else {
			struct string name = get_str(8);
			rv = gfss_tsc(f, &name, "+-=,)")
			trim_tw(name);
			if(!*sv[cdg].contains(name)){
				*sv[cdg][name] = 0;
			}
			switch(rv){
			case ')':
				fungetc(rv, f);
			case ',':
				*sv[cdg][name] += 1;
				continue;
			case '+':
				int a;
				fscanf(f, "%d", &a);
				*sv[cdg][name] += a;
				continue;
			case '-':
				int a;
				fscanf(f, "%d", &a);
				*sv[cdg][name] -= a;
				continue;
			case '=':
				int a;
				fscanf(f, "%d", &a);
				*sv[cdg][name] = a;
				continue;
			}
		}
	}
	if(sv.empty()){
		printf("No story variables found to parse.");
	}
	return rv;
}

void
remove_marker(FILE* f, struct story* s){
	struct string marker = get_string(8);
	gfss_tsc(f, marker, '\n');
	s->tag_map.erase(marker);
}

void
define_marker(FILE* f, umap* db){
	struct string* change = get_string(8)
	gfss_tsc(f, change, ':');
	struct string* marker = get_string(8);
	gfss_tsc(f, marker, '\n');
	um_insert(&style.umap, marker, change); 
}

struct nexus*
get_nxs(){
	struct nexus* n = new *n();
	n->visits = 0;
	n->t = NXS;
	return n;
}

struct exit_group
get_ext_grp(){
	struct exit_group eg;
	//In the future this will initialize the vector and umaps
	return eg;
}

int
parse_ext_grp(FILE* f, struct exit_group* eg){

	struct string e = get_string(8);
	while('\n' != fmove_pw_sl(f)){
		rv = gfss_tsc(f, &e, "\n]");
		if(rv == ']'){
			break;
		}
		eg->exits.push_back(e);
	}
}

void
parse_nexus(FILE* f, story* s){
	struct string tag = gfss_tsc(f, ':');
	if(s->tag_map.contains(tag)){
		fprintf(stderr,
			"Second declaration of nexus tag <%s>", 
			tag.text
		);
		exit(1);
	}

	struct nexus* nxs = get_nxs();
	s->tag_map[tag] = nxs;

	struct exit_group eg = get_ext_grp();
	while(']' != fmove_pw(f) && 
			']' != parse_sv(f, &eg.conds, ":]")
	){
		fmove_nl(f);
		parse_ext_grp(f, &eg);
		nxs->exits.push_back(eg);
		eg = get_ext_group();
	}
	if(nxs->exits.size() == 0){
		printf("Nexus(%s) declared with zero exits.", tag.text);
	}
}

void
free_nxs(struct nexus* nxs){
	delete nxs;
}

struct story*
parse_story_file(const char* filepath, char* start_chapter){
	FILE* f = fopen(filepath, "r");
	if(f == NULL){
		perror(filepath);
		exit(1);
	}

	struct story* story = new *story();
	story->location = start_chapter;

	struct styling style;
	struct story_vars option_conds;

	int c;
	while(EOF != (c = fgetc(f)){
		if(isspace(c)||c == '_'||c == '|'){
			continue;
		}
		switch(c){
		case '/':
			fmove_pc(f, '\n'))
			continue;
		case '\\':
			remove_marker(f, &style.markers);
			continue;
		case '#':
			define_marker(f, &style.markers);
			continue;
		case '@':
			create_data_group(f, story);
			continue;
		case '&':
			add_dg_alias(f, story);
			continue;
		case '[':
			parse_nexus(f, story);
			continue;
		case '(':
			parse_sv(f, &option_conds, ')');
			continue;
		case '{':
			parse_option(f, story);
			continue;
		default:
			parse_paragraph(f, story);
			option_conds.clear();
		}
	}
	if(EOF == fclose(f)){
		fprintf(stderr, "Failed to close story file.");
		exit(1);
	}
	return story;
}

void
free_story(struct story* s){
	for(const auto& [key, value] : s.tag_map){
		free_string(key);
		if(value->t == NXS){
			free_nxs(value);
		} else if(value->t == PRG) {
			free_prg(value);
		}	
	}
}



//OLD CODE:

//Creates a substring from the given position to any of the given ends.
//End will not be included in returned string.
//
//iterator will be moved to the character directly after the given end.
std::string
create_substring(std::string& source, int& c, std::string ends, bool lower){
	std::string string;
	size_t end = source.find_first_of(ends, c);
	if(end == std::string::npos){
		string = trim_string(source.substr(c));
		c = source.length();
	}else{
		string = trim_string(source.substr(c, end - c));
		c = end + 1;
	}
	if(lower){
		string = lowercase_string(string);
	}

	return string;
}

//To be called at newline to check if there are any characters between
//the current c pos and the next newline.
//
//This functions does not move c.
bool
check_double_newline(std::string& story, int c){
	while(c < story.length()){
		if(story[c] == '\n'){
			return true;
		}
		if(std::isspace(story[c])){
			c += 1;
			continue;
		}else{
			return false;
		}
	}
	return false;
}

//All end characters should also be included in delimiters.
//
//Created strings will be lowercased.
//Numbers must be connected to be parsed correctly, meaning 0 whitespace.
story_variables
parse_story_variables(std::string& story, int& c, std::string delimiters, std::string end){
	
	std::vector<std::string> conditions;
	story_variables variables;

	bool _break = false;
	while(c < story.length()){
		conditions.push_back(create_substring(story, c, delimiters, true));
		for(char e : end){
			if(story[c - 1] == e){
				_break = true;
				break;
			}
		}
		if(_break){break;}
	}

	for(auto& s : conditions){

		if(s == "default"){
			return story_variables();

		}else if(s.find_first_of("-+1234567890") == std::string::npos){

			variables.decisions.push_back(s);

		}else{

			int n_begin = s.find_first_of("-+1234567890");
			int n_end = s.find_first_not_of(" -+1234567890", n_begin);

			std::string numbers = s.substr(n_begin, n_end - n_begin);
			numbers = trim_string(numbers);
			if(numbers[0] == '+'){
				numbers.erase(0, 1);
			}

			int amount;
			std::from_chars(numbers.data(), numbers.data() + numbers.length(), amount);

			//if/when revisited, be wary that n_end is moved by create_substring
			std::string amount_of = create_substring(s, n_end, "", true);
			if(amount_of.find_first_of("-+1234567890") != std::string::npos){
				debug("Some numbers may have been cut off within variable condition: " << s << '\n');
			}
			variables.stats_and_items.push_back({amount_of, amount});
		}
	}
	return variables;
}

void
styling::add_marker(std::string marker, std::pair<char, bool> meaning){
	auto& bm = this->bold_markers;
	auto& im = this->italic_markers;

	if(meaning.first == 'b'){

		if(!bm.contains(marker)){
			if(im.contains(marker)){
				debug("Style marker " << marker << "defined for both bold and italic.\n");
			}

			bm.emplace(marker, meaning.second);
		}else{
			debug("Cancelled redefinition of bold marker: " << marker << '\n');
		}

	}else if(meaning.first == 'i'){

		if(!im.contains(marker)){
			if(bm.contains(marker)){
				debug("Style marker " << marker << "defined for both bold and italic.\n");
			}

			im.emplace(marker, meaning.second);
		}else{
			debug("Cancelled redefinition of italic marker " << marker << '\n');
		}
	}

	return;
}

void
styling::remove_marker(std::string marker){
	this->italic_markers.erase(marker);
	this->bold_markers.erase(marker);
}

void
check_for_markers(
	styling& s, 
	glyph_styling& g, 
	std::string& story, int& c
){
	for(auto& marker : s.bold_markers){
		auto pos = story.begin() + c;
		std::string_view sv(pos, pos + marker.first.length());
		if(marker.first == sv){
			g.bold = marker.second;
			c += marker.first.length();
		}
	}
	for(auto& marker : s.italic_markers){
		auto pos = story.begin() + c;
		std::string_view sv(pos, pos + marker.first.length());
		if(marker.first == sv){
			g.italic = marker.second;
			c += marker.first.length();
		}
	}
}

void
set_global_visual(
	std::string& story, int& c, 
	glyph_styling& glyph_style, 
	styling& style
){

	std::string visual = create_substring(story, c, ":", true);
	std::string filename = create_substring(story, c, "\n", true);
	//TODO: Surely there is a better way to write this
	if(visual == "font"){
		glyph_style.font = filename;
	}else if(visual == "option_border"){
		style.option.border = filename;
	}else if(visual == "option_entry_anim"){
		style.option.entry_anim = filename;
	}else if(visual == "option_exit_anim"){
		style.option.exit_anim = filename;
	}else if(visual == "paragraph_border"){
		style.paragraph.border = filename;
	}else if(visual == "paragraph_entry_anim"){
		style.paragraph.entry_anim = filename;
	}else if(visual == "paragraph_exit_anim"){
		style.paragraph.exit_anim = filename;
	}else{
		debug("Undefined visual setting: " << visual << ': ' << filename << '\n');
	}
}

nexus
parse_nexus(std::string& story, int& c){
	nexus nexus;

	while(c < story.length()){
		nexus::exit_group exit_group;
		debug("Creating exit group.\n");

		exit_group.conditions = parse_story_variables(story, c, ",:]", ":]");

		if(story[c - 1] == ']'){
			debug("Nexus closed with empty exit group.\n");
			break;
		}else if(move_iterator_past_whitespace(story, c); story.at(c) == ']'){
			debug("Nexus closed with empty exit group.\n");
			c += 1;
			break;
		}

		while(c < story.length()){
			auto exit = create_substring(story, c, "\n]", true);
			exit_group.exits.push_back(exit);
			debug(exit << '\n');

			if(story[c - 1] == '\n' && check_double_newline(story, c)){
				break;
			}else if(story[c - 1] == ']'){
				break;
			}else{
				move_iterator_past_whitespace(story, c);
				if(story.at(c) == ']'){
					c += 1;
					break;
				}
			}
		}

		nexus.exits.push_back(exit_group);

		if(story[c - 1] == ']'){
			break;
		}else if(move_iterator_past_whitespace(story, c); story[c] == ']'){
			break;
		}
	}

	return nexus;
}

void
parse_option(paragraph& par, std::string& story, int& c, 
	story_variables& option_conditions, glyph_styling& glyph_style, styling& style, 
	std::vector<glyph>& used_glyphs){

	std::string dstlabel = create_substring(story, c, "}", true);
	option option(dstlabel, style.option);

	if(!option_conditions.empty()){
		option.reveal_conditions = option_conditions;
	}

	move_iterator_past_whitespace(story, c);\
	if(story[c] == '('){
		c += 1;
		option.changes = parse_story_variables(story, c, ",)", ")");
		move_iterator_past_whitespace(story, c);
	}

	auto local_g_style = glyph_style;
	while(c < story.length()){
		if(story[c] == '\n'){
			break;
		}

		check_for_markers(style, local_g_style, story, c);

		if(story[c] == ' ' && option.prompt.back().unicode == ' '){
			c += 1;
		}else{
			glyph glyph(story[c], glyph_style);

			if(std::find(used_glyphs.begin(), used_glyphs.end(), glyph) == used_glyphs.end()){
				used_glyphs.push_back(glyph);
			}

			option.prompt.push_back(glyph);
			c += 1;
		}
	}
	while(option.prompt.back().unicode == ' '){
		option.prompt.pop_back();
	}

	par.options.push_back(option);
}

void
story::parse_story_file(std::filesystem::path path){

	std::string story = open_txt_file(path);

	styling style;
	glyph_styling glyph_style;
	story_variables option_conditions;

	int c = 0;
	bool undefine = false;
	while(c < story.length()){
		switch(story[c]){
		case ' ': {
			c += 1;
			continue;
		}
		//Just visually pleasing to be able to use underscores for seperation.
		case '_': {
			int non_underscore = story.find_first_not_of(" _", c);
			int newline = story.find('\n', c);
			if(newline == non_underscore){
				c = newline;
			}else{
				c += 1;
			}
			continue;
		}
		//For inline commenting
		case '/': {
			c = story.find('\n', c) + 1;
			continue;
		}
		case '\n': {
			option_conditions.clear();
			undefine = false;
			c += 1;
			continue;
		}
		case '\\': {
			undefine = true;
			c += 1;
			continue;
		}
		case '#': {
			c += 1;

			if(undefine){

				style.remove_marker(create_substring(story, c, "\n", true));

			}else{

				std::string marker = create_substring(story, c, ":", true);
				std::string meaning = create_substring(story, c, "\n", true);
				std::pair<char, bool> m;
				if(meaning.length() == 0){
					debug("Marker \"" << marker << "\" meaning is empty.\n");
				}else{
					m.second = true;
					if(meaning.at(0) == '!'){
						m.second = false;
						meaning.erase(0);
					}
					if(meaning == "bold"){
						m.first = 'b';
					}else if(meaning == "italic"){
						m.first = 'i';
					}else{
						debug("Undefined meaning for marker \"" << marker << "\": " << meaning << '\n');
					}
				}
				style.add_marker(marker, m);

			}

			continue;

		}
		case '|': {
			c += 1;
			set_global_visual(story, c, glyph_style, style);
			continue;
		}
		case '[': {
			c += 1;
			std::string tag = create_substring(story, c, ":", true);
			move_iterator_past_whitespace(story, c);

			if(this->tag_map.contains(tag)){
				debug("Nexus already exists:" << tag << '\n');
			}else{
				
				int vector_pos = this->nexuses.size();
				this->tag_map.emplace(tag, std::pair('n', vector_pos));

				this->nexuses.push_back(parse_nexus(story, c));
				
			}

			continue;
		}
		case '(': {
			c += 1;
			option_conditions = parse_story_variables(story, c, ",)", ")");
			continue;
		}
		case '{': {
			c += 1;
			std::string tag = create_substring(story, c, ":", true);
			debug("Option tag: " << tag << '\n');
			if(!this->tag_map.contains(tag)){
				debug("Attempted to attach option to undefined paragraph: " << tag << '\n');
			}else if(this->tag_map[tag].first == 'n'){
				debug("Attempted to attach option to nexus: " << tag << '\n');

			}else{

				auto& par = this->paragraphs.at(this->tag_map.at(tag).second);
				parse_option(par, story, c, option_conditions, glyph_style, style, used_glyphs);
				
			}
			continue;
		}
		default: {
			std::string tag = std::string(create_substring(story, c, ":", true));
			debug("Creating paragraph \"" << tag << "\"\n");

			if(this->tag_map.contains(tag)){
				if(this->tag_map.at(tag).first == 'n'){
					debug("Paragraph tag already exists as a nexus: " << tag << '\n');
				}else{
					debug("Paragraph tag " << tag << " already exists.\n");
				}

			}else{

				int vector_pos = this->paragraphs.size();
				this->tag_map.emplace(tag, std::pair('p', vector_pos));

				paragraph paragraph(style.paragraph);

				move_iterator_past_whitespace(story, c);
				if(story[c] == '('){
					c += 1;
					paragraph.changes = parse_story_variables(story, c, ",)", ")");
					move_iterator_past_whitespace(story, c);
				}

				auto local_g_style = glyph_style;
				while(c < story.length()){
					//TODO: implement style marker reading
					if(story[c] == '\n'){
						c += 1;
						if(check_double_newline(story, c)){
							while(paragraph.text[paragraph.text.size() - 1].unicode == ' '){
								paragraph.text.pop_back();
							}
							break;
						}else{
							if(paragraph.text.back().unicode != ' '){
								paragraph.text.push_back(glyph(' ', glyph_style));
							}
							move_iterator_past_whitespace(story, c);
						}
						continue;
					}
					
					check_for_markers(style, local_g_style, story, c);
					
					if(story[c] == ' ' && paragraph.text.back().unicode == ' '){
						c += 1;
					}else{
						glyph glyph(story[c], glyph_style);

						if(std::find(used_glyphs.begin(), used_glyphs.end(), glyph) == used_glyphs.end()){
							used_glyphs.push_back(glyph);
						}

						paragraph.text.push_back(glyph);
						c += 1;
					}
				}
				while(paragraph.text.back().unicode == ' '){
					paragraph.text.pop_back();
				}

				this->paragraphs.push_back(paragraph);
			}
			continue;
		}
		}
	}
}

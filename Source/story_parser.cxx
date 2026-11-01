#include "story_parser.h"
#include "utils.h"

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
			ungetc(c, f);
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

	} else if(pos > 0){
		return -1;
	}
	return -2;
}

int
list_contains_c(const char* list, char c){
	for(int i = 0; list[i] != '\0'; i++){
		if(c == list[i]){
			return 1;
		}
	}
	return 0;
}

int
gss_tsc(const char* start, struct string* s, char* end){
	int c;
	while('\0' != (c = *start++)){
		if(list_contains_c(end, c)){
			break;
		}
		dynamic_append(s, c);
	}
	if(c == '\n'){
		trim_tw(s);
	}
	return c;
}

//gfss_tsc == "Get file substring to single char"
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
gfss_tsc(FILE* f, struct string* string, const char* end){
	int c;
	while(EOF != (c = fgetc(f))){
		if(list_contains_c(end, c)){
			break;
		}
		dynamic_append(string, c);
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
			dynamic_sappend(string, end, 0, ki);
			ki = 0;
			break;
		case 0:
			ki += 1;
			continue;
		case 1:
			goto finished;
		}
		
		dynamic_append(string, c);
	}
finished:
	if(c == '\n'){
		trim_tw(string);
	} else if(c == EOF) {
		return EOF;
	}
	return 1;
}

int
use_fint(FILE* f, int* trg, int op){
	int a;
	fscanf(f, "%8d", &a);
	switch(op){
	case '+':
		*trg += a;
		return fgetc(f);
	case '-':
		*trg -= a;
		return fgetc(f);
	case '=':
		*trg = a;
		return fgetc(f);
	}
	fprintf(stderr, "Invalid operation type: \"%c\"\n", op);
}

void
create_data_group(FILE* f, struct story* s){
	struct string name = get_string(8);
	gfss_tsc(f, &name, "\n");
	data_group dg;
	s->data_groups[name] = dg;
}

void
add_dg_alias(FILE* f, struct story* s){
	struct string dg = get_string(8);
	if('\n' == gfss_tsc(f, &dg, ":\n")){
		printf("dg alias ended at \\n before finding the alias: %s", dg.text);
		return;
	}
	struct string alias = get_string(8);
	gfss_tsc(f, &alias, "\n");
	if( s->dg_aliases.contains(alias) ){
		fprintf(stderr, "Attempted to redefine dg alias \"%s\"\n", alias.text);
		exit(1);
	}
	s->dg_aliases[alias] = dg;
}

int
add_dg_var(FILE* f, data_group* dg, const char* ends){
	//TODO: This is gonna run a lot of times,
	//figure out a way to only call mappend once
	struct string dne = get_string(8);
	//TODO: Figure out a way to chain mappend
	dynamic_mappend(&dne, "+-=");
	dynamic_mappend(&dne, ends);

	struct string name = get_string(8);
	int rv = gfss_tsc(f, &name, dne.text);
	trim_tw(&name);
	free_string(&dne);

	if(!dg->contains(name)){
		dg->at(name) = 0;
	}
	if(list_contains_c(ends, rv)){
		dg->at(name) += 1;
		return rv;
	}
	return use_fint(f, &dg->at(name), rv);
}

void
remove_marker(FILE* f, struct styling* s){
	struct string marker = get_string(8);
	gfss_tsc(f, &marker, "\n");
	if(s->markers.contains(marker)){
		auto i = s->markers.find(marker);
		auto n = s->markers.extract(i);
		free_string(&n.key());
		free_string(&n.mapped());
	}
	free_string(&marker);
}

void
define_marker(FILE* f, struct styling* db){
	struct string change = get_string(8);
	gfss_tsc(f, &change, ":");
	struct string marker = get_string(8);
	gfss_tsc(f, &marker, "\n");
	db->markers[marker] = change;
	//TODO: Update how markers are stored so one marker
	//		can apply multiple changes.
}

int
edit_local_sv(FILE* f, story_vars* sv, char* ends){
	int rv;
	struct string cdg = get_string(8);
	
	while(!list_contains_c(ends, rv = fmove_pw(f))){

		if(rv == '<'){
			gfss_tsc(f, &cdg, ">");
			if(!sv->contains(cdg)){
				struct string n = duplicate_string(&cdg);
				data_group dg;
				sv->insert({n, dg});
			}
		} else if(cdg.len == 0) {
			printf("Active data group not declared during sv parse.");
			return -1;
		} else if(rv == ','){
			fseek(f, 1, SEEK_CUR);
		} else {
			if(')' == add_dg_var(f, &sv->at(cdg), ",)")){
				break;
			}
		}
	}
	if(sv.empty()){
		printf("No story variables found to parse.");
	}
	free_string(cdg);
	return rv;
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
		if(e.len > 0){
			eg->exits.push_back(e);
		}
		if(rv == ']'){
			break;
		}
		e = get_string(8);
	}
	if(e.len == 0){
		free_string(e);
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
			']' != edit_local_sv(f, &eg.conds, ":]")
	){
		fmove_pw(f);
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

#define TOKEN_LENGTH 4
/*
|bold(1)
|itlc(1)
|font(arial)
|rgba(255,255,255,255)
|size(24)
|pg_b(intricate)
|op_b(intricate)
*/

long
parse_change_number(struct string* s){
	char* t = s->text;
	if(t[TOKEN_LENGTH] != '('){
		fprintf(stderr, "Invalid change: %s\n", t);
		exit(1);
	}
	char* end;
	long v = strtol(t+TOKEN_LENGTH+1, &end, 10);
	if(*end != ')'){
		fprintf(stderr, "Invalid change: %s\n", t);
		exit(1);
	}
	return v;
}

SDL_Color
parse_rgba_change(struct string* s){
	char* t = s->text;
	if(t[TOKEN_LENGTH] != '('){
		fprintf(stderr, "Invalid change: %s\n", t);
		exit(1);
	}
	struct SDL_Color clr;
	char* end;
	//TODO: This can probably be done better
	clr.r = strtol(t+TOKEN_LENGTH+1, &end, 10);
	clr.g = strtol(end+1, &end, 10);
	clr.b = strtol(end+1, &end, 10);
	clr.a = strtol(end+1, &end, 10);
	if(*end != ')'){
		fprintf(stderr, "Invalid change: %s\n", t);
		exit(1);
	}
	return clr;
}

std::string
parse_change_text(struct string* s){
	char* t = s->text;
	if(t[TOKEN_LENGTH] != '('){
		fprintf(stderr, "Invalid change: %s\n", t);
		exit(1);
	}
	struct string c = get_string(8);
	if('\n' == gss_tsc(t+TOKEN_LENGTH+1, &c, ")\n")){
		fprintf(stderr, "Invalid change; missing close parenthesis: %s\n", t);
	}
	//This is only until glyphtext gets updated to struct string
	std::string cs(c.text);
	free_string(c);
	return cs;
}

void
update_styling(const struct styling* s, struct string* change){
	char* t = change->text;
	if(0 == strncmp(t, "bold", TOKEN_LENGTH)){
		s->glyph.bold = !!parse_change_number(change);
	} else if(strncmp(t, "itlc", TOKEN_LENGTH) {
		s->glyph.italic = !!parse_change_number(change);
	} else if(strncmp(t, "font", TOKEN_LENGTH) {
		s->glyph.font = parse_change_text(change);
	} else if(strncmp(t, "rgba", TOKEN_LENGTH) {
		s->glyph.color = parse_rgba_change(change);
	} else if(strncmp(t, "size", TOKEN_LENGTH) {
		s->glyph.size = parse_change_number(change);
	} else if(strncmp(t, "pg_b", TOKEN_LENGTH) {
		s->paragraph.border = parse_change_text(change);
	} else if(strncmp(t, "op_b", TOKEN_LENGTH) {
		s->option.border = parse_change_text(change);
	} else {
		fprintf(stderr, "INVALID CHANGE \"%s\"\n", change->text);
		exit(1);
	}
}

int
fread_glyphtext(
	FILE* f,
	vector<glyph>* cntr, struct styling* s,
	char* ends
){
	bool dnt = false; 
	if( 0 == strncmp(ends, "\n\n", 2) ){
		dn = true;
		ends += 2;
	}
	int c;
	struct string cache = get_string(8);
	while( EOF != (c = fgetc(f))){
		if(dnt){
			if(c == '\n' && '\n' == fmove_pw_sl(f) ){
				break;
			}
		}
		if(list_contains_c(ends, c)){
			break;
		}
		dynamic_append(cache, c);
		for(const auto& [marker, change] : s->markers){
			int i = 0;
			for( int rv = 0; -1 != (rv = match_key(
					cache.text[cache.len - marker.len + i],
					marker, i));
					i += 1
			){
				if(rv == 1){
					cntr->resize(cntr->size() - marker.len);
					if( 0 != strncmp(change.text, "nwln", 4) ){
						update_styling(s, change);
					} else {
						cntr->push_back(glyph('\n', s));
					}
					break;
				}
			}	
		}
		glyph g;
		g.unicode = c;
		g.style = s;
		cntr->push_back(g);
	}
	return c;
}

void
parse_sv_change(FILE* f, struct string* dg, struct data_change* dc){

	dc->dg = duplicate_string(dg);
	dc->var = get_string(8);
	int c = gfss_tsc(f, &dc->var, "+-=,)");
	trim_tw(&dc->var);
	
	if(c == ',' || c == ')'){
		fprintf(stderr, "No change operator found for change to variable \"%s\" in data group \"%s\"\n",
			dc->var.text, dg->text
		);
	}
	dc->op = c;
	fscanf(f, "%8d", &dc->val);

}

void
parse_sv_changes(FILE* f, sv_changes* svc){
	int c;
	struct string cdg = get_str(8);
	
	while( ')' != (c = fmove_pw(f)) ){

		if(c == '<'){
			clear_string(cdg);
			if(')' == (gfss_tsc(f, &cdg, ")>"))){
				fprintf(stderr, "sv changes closed while changing data group: \"%s\"\n", cdg.text);
				exit(1)
			}
		} else if(cdg.len == 0) {
			printf("Active data group not declared during sv parse.");
			exit(1);
		} else if(rv == ','){
			fseek(f, 1, SEEK_CUR);
		} else {
			struct data_change dc;
			parse_sv_change(&cdg, &dc, f);
		}
	}
	if(sv.empty()){
		printf("No story variables found to parse.");
	}
	free_string(cdg);
	return rv;
}

}

void
parse_option(
	FILE* f, struct story* s,
	struct styling* style, story_vars* conds
){
	struct option o;
	o.reveal_conds = *conds;
	o.txtr_data = style.option;

	struct string base = get_string(8)
	fmove_pw(f);
	if('}' == gfss_tsc(f, &base, ":}"){
		fprintf(stderr, "No destination found for option on paragraph \"%s\".\n", base.text);
		exit(1);
	}

	fmove_pw(f);
	struct string dst = get_string(8);
	gfss_tsc(f, &dst, "}");

	trim_tw(&base);
	trim_tw(&dst);

	if( '(' == fmove_pw(f) ){
		fseek(f, 1, SEEK_CUR);
		parse_sv_changes(f, &o.changes);
		fmove_pw(f);
	}

	fread_glyphtext(f, &o.prompt, style, "\n");
	s->used_glyphs.insert(s->used_glyphs.end(),
		o.prompt.begin(), o.prompt.end()
	);

	if(s->tag_map.contains(base) && s->tag_map[base]->t == PRG){
		s->tag_map[base]->options.push_back(o);
	} else {
		fprintf(stderr, "Attempted to attach option to undeclared paragraph: \"%s\"\n", base.text);
		exit(1);
	}
}

void
parse_paragraph(
	FILE* f, struct story* s,
	struct styling* style
){
	struct paragraph p;
	p.txtr_data = style.paragraph;

	struct string name = get_string(8);
	gfss_tsc(f, &name, ":");
	trim_tw(&name);

	if( '(' == fmove_pw(f) ){
		fseek(f, 1, SEEK_CUR);
		parse_sv_changes(f, &p.changes);
		fmove_pw(f);
	}

	fread_glyphtext(f, &p.text, style, "\n\n");
	s->used_glyphs.insert(s->used_glyphs.end(),
		p.text.begin(), p.text.end()
	);

	if(s->tag_map.contains(name)){
		fprintf(stderr, "Attempted to store paragraph with a name already reserved by another location: \"%s\"\n", name.text);
		exit(1);
	}

	s->tag_map[name] = p;
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
		case '|':
			struct string alteration = get_string(8);
			gfss_tsc(f, &alteration, "\n");
			update_styling(&style, &alteration);
			free_string(alteration);
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
			edit_local_sv(f, &option_conds, ')');
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


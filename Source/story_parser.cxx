#include "story_parser.h"
#include "utils.h"

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

//File move past char
//Returns false on EOF
bool
fmove_nl(tFILE* file){
	char c;
	while(EOF != (c = tfgetc(file))){
		if(c == '\n'){
			return true;
		}
	}
	return false;
}

//File move past whitespace
//Returns -1 on EOF
//Otherwise returns the first non-ws char
int
fmove_pw(tFILE* f){
	int c;
	while(EOF != (c = tfgetc(f))){
		if(!isspace(c)){
			tungetc(c, f);
			return c;
		}
	}
	return false;
}

int
fmove_pw_sl(tFILE* f){
	int c;
	while(EOF != (c = tfgetc(f))){
		if( !isspace(c) || c == '\n' ){
			tungetc(c, f);
			return c;
		}
	}
	return EOF;
}

int
ffind_c(tFILE* f, const char* ends){
	int c;
	while( EOF != (c = tfgetc(f)) ){
		if(list_contains_c(ends, c)){
			tungetc(c, f);
			return c;
		}
	}
	return EOF;
}

int
gss_tsc(const char* start, struct zstring* s, const char* ends){
	int c;
	while('\0' != (c = *start++)){
		if(list_contains_c(ends, c)){
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
gfss_tsc(tFILE* f, struct zstring* string, const char* end){
	int c;
	while(EOF != (c = tfgetc(f))){
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

void
create_data_group(tFILE* f, struct story* s){
	struct zstring name = get_string(8);
	gfss_tsc(f, &name, "\n");
	data_group dg;
	s->data_groups[name] = dg;
}

void
add_dg_alias(tFILE* f, struct story* s){
	struct zstring dg = get_string(8);
	if( '\n' == gfss_tsc(f, &dg, ":\n") ){
		printf("dg alias ended at \\n before finding the alias: %s", dg.text);
		return;
	}
	struct zstring alias = get_string(8);
	gfss_tsc(f, &alias, "\n");
	if( s->dg_aliases.contains(alias) ){
		fprintf(stderr, "Attempted to redefine dg alias \"%s\"\n", alias.text);
		exit(1);
	}
	s->dg_aliases[alias] = dg;
}

void
remove_marker(tFILE* f, struct parse_rundata* prd){
	struct zstring marker = get_string(8);
	gfss_tsc(f, &marker, "\n");
	if(prd->markers.contains(marker)){
		// TODO: Fix memory leak
		prd->markers.erase(marker);
	}
	free_string(&marker);
}

void
define_marker(tFILE* f, struct parse_rundata* prd){
	struct zstring change = get_string(8);
	gfss_tsc(f, &change, ":");
	struct zstring marker = get_string(8);
	gfss_tsc(f, &marker, "\n");
	prd->markers[marker] = change;
	//TODO: Update how markers are stored so one marker
	//		can apply multiple changes.
}

void
parse_sv_change(tFILE* f, struct zstring* dg, struct data_change* dc, struct zstring* ends){

	dc->dg = duplicate_string(dg);
	dc->var = get_string(8);
	int c = gfss_tsc(f, &dc->var, ends->text);
	trim_tw(&dc->var);

	if( NULL == strchr("><-=+", c) ){
		fprintf(stderr, "No change operator found for change to variable \"%s\" in data group \"%s\"\n",
			dc->var.text, dg->text
		);
		exit(1);
	}
	dc->op = c;
	//TODO: THIS MIGHT CONSUME NEWLINES AND MESS UP LINE TRACKING
	if(1 != fscanf(f->data, " %8d", &dc->val)){
		fprintf(stderr, "(LINE %d)Unexpected data in input stream", f->line);
	}

}

void
set_cdg(zstring* cdg, tFILE* f, zstring* e){
	clear_string(cdg);
	if( '>' != ( gfss_tsc(f, cdg, e->text) ) ){
		fprintf(stderr, "(LINE %d)sv changes closed while changing data group: \"%s\"\n", f->line, cdg->text);
		exit(1);
	}
	trim_tw(cdg);
}

bool
check_default(tFILE* f){
	fmove_pw(f);
	const char* d = "default";	
	char buffer[8] = {0};
	int i = 0;
	while( (buffer[i] = tfgetc(f)) == d[i] ){
		i += 1;
	}

	if(0 == strncmp(d, buffer, 7)){
		return true;
	} else {
		while(i >= 0){
			tungetc(buffer[i--], f);
		}
		return false;
	}
}

int
parse_sv_changes(tFILE* f, sv_changes* svc, const char* ends){
	
	int c;
	if(check_default(f)){
		c = ffind_c(f, ends);
		(void)tfgetc(f);
		goto no_changes;
	}

	{
	struct zstring e = get_string(8);
	dynamic_mappend(&e, "><-=+");
	dynamic_mappend(&e, ends);

	struct zstring cdg = get_string(8);
	while( EOF != (c = fmove_pw(f)) ){
		if( list_contains_c(ends, c) ){
			(void)tfgetc(f);
			break;
		}

		if(c == '<'){
			(void)tfgetc(f);
			set_cdg(&cdg, f, &e);
		} else if(cdg.len == 0) {
			printf("(LINE %d)Active data group not declared during sv parse.", f->line);
			exit(1);
		} else if(c == ','){
			(void)tfgetc(f);
		} else {
			struct data_change dc;
			parse_sv_change(f, &cdg, &dc, &e);
			svc->push_back(dc);
		}
	}
	if(svc->empty()){
		printf("(LINE %d)No story variables found to parse.", f->line);
	}
	free_string(&cdg);
	}

no_changes:
	return c;
}

struct nexus*
get_nxs(){
	struct nexus* n = new nexus();
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

void
parse_ext_grp(tFILE* f, struct exit_group* eg){

	struct zstring e = get_string(8);
	while('\n' != fmove_pw_sl(f)){
		int c = gfss_tsc(f, &e, "\n]");
		if(e.len > 0){
			eg->exits.push_back( duplicate_string(&e) );
		}
		if(c == ']'){
			tungetc(c, f);
			break;
		}
		clear_string(&e);
	}
	free_string(&e);
}

void
parse_nexus(tFILE* f, story* s){
	struct zstring tag = get_string(8);
	gfss_tsc(f, &tag, ":");
	trim_tw(&tag);
	if(s->tag_map.contains(tag)){
		fprintf(stderr,
			"(LINE %d)Second declaration of nexus tag <%s>",
			f->line, tag.text
		);
		exit(1);
	}

	struct nexus* nxs = get_nxs();
	s->tag_map[tag] = nxs;

	int c;
	struct exit_group eg = get_ext_grp();
	while(']' != (c = fmove_pw(f)) && c != '\0' &&
		']' != parse_sv_changes(f, &eg.conds, ":]")
	){
		fmove_pw(f);
		parse_ext_grp(f, &eg);
		nxs->exits.push_back(eg);
		eg = get_ext_grp();
	}
	if(nxs->exits.size() == 0){
		printf("(LINE %d)Nexus \"%s\" declared with zero exits.", f->line, tag.text);
	}
	(void)tfgetc(f);
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
parse_styling_number(struct zstring* s){
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
parse_rgba_change(struct zstring* s){
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
parse_change_text(struct zstring* s){
	char* t = s->text;
	if(t[TOKEN_LENGTH] != '('){
		fprintf(stderr, "Invalid change: %s\n", t);
		exit(1);
	}
	struct zstring c = get_string(8);
	if('\n' == gss_tsc(t+TOKEN_LENGTH+1, &c, ")\n")){
		fprintf(stderr, "Invalid change; missing close parenthesis: %s\n", t);
	}
	//This is only until glyphtext gets updated to struct zstring
	std::string cs(c.text);
	free_string(&c);
	return cs;
}

void
update_styling(struct parse_rundata* s, struct zstring* change){
	char* t = change->text;
	if(0 == strncmp(t, "bold", TOKEN_LENGTH)){
		s->glyph.bold = !!parse_styling_number(change);
	} else if( strncmp(t, "itlc", TOKEN_LENGTH) ) {
		s->glyph.italic = !!parse_styling_number(change);
	} else if( strncmp(t, "font", TOKEN_LENGTH) ) {
		s->glyph.font = parse_change_text(change);
	} else if( strncmp(t, "rgba", TOKEN_LENGTH) ) {
		s->glyph.color = parse_rgba_change(change);
	} else if( strncmp(t, "size", TOKEN_LENGTH) ) {
		s->glyph.size = parse_styling_number(change);
	} else if( strncmp(t, "pg_b", TOKEN_LENGTH) ) {
		s->paragraph.border = parse_change_text(change);
	} else if( strncmp(t, "op_b", TOKEN_LENGTH) ) {
		s->option.border = parse_change_text(change);
	} else {
		fprintf(stderr, "INVALID CHANGE \"%s\"\n", change->text);
		exit(1);
	}
}

int
fread_glyphtext(
	tFILE* f,
	glyphtext* cntr, struct parse_rundata s,
	const char* ends
){
	//ALWAYS PLACE DOUBLE NEWLINE AT THE BEGINNING OF THE ENDS PARAMETER
	bool dnt = false;
	if( 0 == strncmp(ends, "\n\n", 2) ){
		dnt = true;
		ends += 2;
	}

	int c;
	struct zstring cache = get_string(8);
	while( EOF != (c = tfgetc(f))){
		if(dnt){
			if(c == '\n' && '\n' == fmove_pw_sl(f) ){
				break;
			}
		}
		if(list_contains_c(ends, c)){
			break;
		}
		dynamic_append(&cache, c);
		for(auto& [marker, change] : s.markers){
			int i = 0;
			for( int rv = 0; -1 != (rv = match_key(
					cache.text[cache.len - marker.len + i],
					marker.text, i));
					i += 1
			){
				if(rv == 1){
					cntr->glyphs.resize(cntr->glyphs.size() - marker.len);
					if( 0 != strncmp(change.text, "nwln", 4) ){
						update_styling(&s, &change);
					} else {
						cntr->glyphs.push_back(glyph('\n', s.glyph));
					}
					break;
				}
			}
		}
		glyph g;
		g.unicode = c;
		g.style = s.glyph;
		cntr->glyphs.push_back(g);
	}
	return c;
}

struct option*
get_option(sv_changes* rc, txtr_info* ti){
	struct option* o = new option();
	o->dst_label = get_string(8);
	return o;
}

void
parse_option(
	tFILE* f, struct story* s,
	struct parse_rundata* prd, sv_changes* conds
){
	struct option* o = get_option(conds, &prd->option);
	o->reveal_conds = *conds;
	o->txtr_data = prd->option;

	struct zstring base = get_string(8);
	trim_tw(&base);
	fmove_pw(f);
	if( '}' == gfss_tsc(f, &base, ":}") ){
		fprintf(stderr, "No destination found for option on paragraph \"%s\".\n", base.text);
		exit(1);
	}

	fmove_pw(f);
	gfss_tsc(f, &o->dst_label, "}");
	trim_tw(&o->dst_label);

	if( '(' == fmove_pw(f) ){
		(void)tfgetc(f);
		parse_sv_changes(f, &o->changes, ")");
		fmove_pw(f);
	}

	fread_glyphtext(f, &o->prompt, *prd, "\n");
	o->prompt.alignment = GLYPHTEXT_ALIGN_CENTER;

	s->used_glyphs.insert(s->used_glyphs.end(),
		o->prompt.glyphs.begin(), o->prompt.glyphs.end()
	);

	if( !s->tag_map.contains(base) ){
		fprintf(stderr, "Attempted to attach option to undeclared paragraph: \"%s\"\n", base.text);
		exit(1);
	} else if (s->tag_map[base]->t != PRG) {
		fprintf(stderr, "Attempted to attach option to something other than a paragraph: \"%s\"\n", base.text);
	}

	struct paragraph* p = (struct paragraph*)s->tag_map[base];
	p->options.push_back(o);
}

void
free_option(struct option* o){
	delete o;
}

struct paragraph*
get_prg(){
	struct paragraph* p = new paragraph();
	p->t = PRG;
	return p;
}

void
parse_paragraph(
	tFILE* f, struct story* s,
	struct parse_rundata* prd 
){
	struct paragraph* p = get_prg();
	p->txtr_data = prd->paragraph;

	struct zstring name = get_string(8);
	gfss_tsc(f, &name, ":");
	trim_tw(&name);

	if( '(' == fmove_pw(f) ){
		(void)tfgetc(f);
		parse_sv_changes(f, &p->changes, ")");
		fmove_pw(f);
	}

	fread_glyphtext(f, &p->text, *prd, "\n\n");
	p->text.alignment = GLYPHTEXT_ALIGN_JUSTIFY;

	s->used_glyphs.insert(s->used_glyphs.end(),
		p->text.glyphs.begin(), p->text.glyphs.end()
	);

	if(s->tag_map.contains(name)){
		fprintf(stderr, "Attempted to store paragraph with a name already reserved by another location: \"%s\"\n", name.text);
		exit(1);
	}

	printf("Defined paragraph \"%s\"\n", name.text);
	s->tag_map[name] = (void*)p;
}

void
free_prg(struct paragraph* p){
	for(auto& o : p->options){
		free_option(o);
	}
	delete p;
}

struct story*
parse_story_file(const char* filepath, char* start_chapter){
	tFILE* f = tfopen(filepath, "r");

	struct story* story = new struct story();

	struct parse_rundata prd;
	sv_changes option_conds;

	int c;
	while( EOF != (c = tfgetc(f)) ){
		if(isspace(c)||c == '_'||c == '|'){
			continue;
		}
		switch(c){
		case '/':
			fmove_nl(f);
			continue;
		case '\\':
			remove_marker(f, &prd);
			continue;
		case '#':
			define_marker(f, &prd);
			continue;
		case '|': {
			struct zstring alteration = get_string(8);
			gfss_tsc(f, &alteration, "\n");
			update_styling(&prd, &alteration);
			free_string(&alteration);
			continue;
		}
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
			parse_sv_changes(f, &option_conds, ")");
			continue;
		case '{':
			parse_option(f, story, &prd, &option_conds);
			continue;
		default:
			tungetc(c, f);
			parse_paragraph(f, story, &prd);
			option_conds.clear();
		}
	}
	if(EOF == tfclose(f)){
		fprintf(stderr, "Failed to close story file.");
		exit(1);
	}

	story->location = get_string(4);
	dynamic_mappend(&story->location, start_chapter);
	if(!story->tag_map.contains(story->location)){
		fprintf(stderr, "Story start index \"%s\" does not exist", start_chapter);
		exit(1);
	}	
	return story;
}

void
free_story(struct story* s){
	/* TODO: Fix memory leak
	for(auto& [key, value] : s->tag_map){
		auto i = s->tag_map.find(key);
		auto node = s->tag_map.extract(i);
		free_string(&node.key());

		struct nexus* n = (struct nexus*)node.mapped();
		LCTN_T t = n->t;
		if(t == NXS){
			free_nxs(n);
		} else if(t == PRG) {
			free_prg((struct paragraph*)n);
		}
	}
	*/
}


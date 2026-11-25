#include "utils.h"

void*
xmalloc(size_t bytes){
	void* m = malloc(bytes);
	if(m == NULL){
		fprintf(stderr, "Out of memory!\n");
		exit(1);
	}
	return m;
}

void*
xcalloc(size_t amount, size_t size){
	void* m = calloc(amount, size);
	if(m == NULL){
		fprintf(stderr, "Out of memory!\n");
		exit(1);
	}
	return m;
}

void*
xrealloc(void* p, size_t new_size){
	void* n = realloc(p, new_size);
	if(n == NULL){
		fprintf(stderr, "Out of memory!\n");
		exit(1);
	}
	return n;
}

struct tFILE*
tfopen(const char* file, const char* flags){
	struct tFILE* tf = (struct tFILE*)xmalloc(sizeof *tf);
	tf->data = fopen(file, flags);
	if(tf->data == NULL){
		perror(file);
		exit(1);
	}
	tf->line = 1;
	return tf;
}

int
tfclose(tFILE* tf){
	int r = fclose(tf->data);
	free(tf);
	return r;
}

int
tfgetc(tFILE* tf){
	int c = fgetc(tf->data);
	if(c == '\n'){
		tf->line += 1;
	}
	//Note: tolower is for this project specifically, 
	//as with how tfgetc is used, this allows for the 
	//story file to easily be case insensitive
	return tolower(c);
}

void
tungetc(int c, tFILE* tf){
	ungetc(c, tf->data);
	if(c == '\n'){
		tf->line -= 1;
	}
}

void
init_string(struct zstring* string, size_t cap){
	string->len = 0;
	string->cap = cap;
	string->text = (char*)xcalloc(cap, sizeof(char));
}

struct zstring
get_string(size_t cap){
	struct zstring s;
	init_string(&s, cap);
	return s;
}

void
free_string(struct zstring* s){
	free(s->text);
}

struct zstring
duplicate_string(struct zstring* o){
	struct zstring n = *o;
	n.text = strdup(o->text);
	return n;
}

void
trim_tw(struct zstring* s){
	while( isspace(s->text[s->len-1]) ){
		s->text[--s->len] = '\0';
	}
}

void
clear_string(struct zstring* s){
	s->text[0] = '\0';
	s->len = 0;
}

void
dynamic_append(struct zstring* string, char c){
	if((string->cap - string->len) <= 1){
		string->text = (char*)xrealloc(string->text, string->cap *= 2);
	}
	string->text[string->len++] = c;
	string->text[string->len] = '\0';
}

void
dynamic_mappend(struct zstring* s, const char* mc){
	for(int i = 0; mc[i] != '\0'; i++){
		dynamic_append(s, mc[i]);
	}
}

void
dynamic_sappend(struct zstring* s, const char* src, int i, int l){
	while(i < (i + l)){
		dynamic_append(s, src[i]);
		i += 1;
	}
}

int randomize_number(int range){
	std::random_device random_device;
	std::mt19937 generator(random_device());
	std::uniform_int_distribution<size_t> distributor(0, range);
	return distributor(generator);
} 

std::string
lowercase_string(std::string text){
	std::string result;
	for(char c : text){
		result.push_back(std::tolower(c));
	}
	return result;
}

std::string
trim_string(std::string string){
	size_t start = string.find_first_not_of(" \n\t\v\f\r");
	if(start == std::string::npos){
		return string;
	}

	int end = string.find_last_not_of(" \n\t\v\f\r");

	return string.substr(start, end - start + 1);
}

void
move_iterator_past_whitespace(std::string& source, int& c){
	while(c < source.length()){
		if(!std::isspace(source.at(c))){
			return;
		}
		c += 1;
	}
}

std::string
open_txt_file(std::filesystem::path path){
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Error opening file: "
			<< std::strerror(errno);
	}
	std::stringstream read_file;
	read_file << file.rdbuf();
	return read_file.str();
}

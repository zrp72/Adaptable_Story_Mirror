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

/*
//The following functions are placeholder
//for future custom umap implementation

	int um_init(struct umap*, um_hashfunc hf){
	
	}
	int um_insert(struct umap* um, 
		void* k, void* v
	){
		size_t i = hf(*k);
		umn* bkt = um->buckets[i];
		while(bkt->next != NULL){
			bkt = bkt->next;
		}
		bkt->value = v;
	}

	void um_remove(umap* um,
		void* k,
	){
		*um.erase(*k);
	}

	bool um_contains(umap* um,
		void* key,
	){
		if(*um.contains(*key)){
			return true;
		}
		return false;
	}

	size_t string_hash(void*){}
	size_t char_hash(void*){}
	size_t int_hash(void*){}
	size_t bool_hash(void*){}//Just in case
////////////////////////////////////////////
////////////////////////////////////////////
*/

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

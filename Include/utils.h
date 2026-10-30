#ifndef ASM_utils
#define ASM_utils 1
#include <string>
#include <filesystem>
#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <cstring>
#include <inttypes>

#define debug(x) std::cout<<x

template<typename x>
using vector = std::vector<x>;

void* xmalloc(size_t bytes);
void* xcalloc(size_t amount, size_t size);
void* xrealloc(void*, size_t new_size);
 
struct umn {
	void* key;
	void* value;
	struct umn* next;
};
typedef (size_t)(*um_hashfunc)(void*);
struct umap {
	/*
	umn** buckets;
	int capacity;
	int hash_count;
	float grow_threshold;

	um_hashfunc hash;
	*/
};
int um_init(struct umap*)
int um_insert(umap*,
	void* key, void* val, 
);
void um_remove(umap*,
	void* key,
);
bool um_contains(umap*,
	void* key,
);
size_t string_hash(void*);
size_t char_hash(void*);
size_t int_hash(void*);
size_t bool_hash(void*);

struct {
	char* text;
	size_t len;
	size_t cap;
} string;
void init_string(struct string, int cap);

std::string lowercase_string(std::string);
std::string trim_string(std::string);
int randomize_number(int);
void move_iterator_past_whitespace(std::string&, int&);
std::string open_txt_file(std::filesystem::path);

#endif

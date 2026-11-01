#ifndef ASM_utils
#define ASM_utils 1

#include <string>
#include <filesystem>
#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cinttypes>

#define debug(x) std::cout<<x

template<typename x>
using vector = std::vector<x>;

void* xmalloc(size_t bytes);
void* xcalloc(size_t amount, size_t size);
void* xrealloc(void*, size_t new_size);

/*
struct umn {
	void* key;
	void* value;
	struct umn* next;
};

typedef (size_t)(*um_hashfunc)(void*);
struct umap {
	umn** buckets;
	int capacity;
	int hash_count;
	float grow_threshold;

	um_hashfunc hash;
};

int um_init(struct umap*);
int um_insert(umap*,
	void* key, void* val 
);
void um_remove(umap*,
	void* key
);
bool um_contains(umap*,
	void* key
);

size_t string_hash(void*);
size_t char_hash(void*);
size_t int_hash(void*);
size_t bool_hash(void*);
*/

struct string {
	char* text;
	size_t len;
	size_t cap;
};
//THIS IS PLACEHOLDER UNTIL FULL C UMAP IMPLEMENTATION
struct string_hash {
	size_t operator()(const string& s) const noexcept {
		return std::hash<std::string_view>{}(s.text);
	}
};
struct string_equal {
	bool operator()(const string& a, const string& b) const noexcept {
		return std::strcmp(a.text, b.text) == 0;
	}
};
struct char_ptr_hash {
	size_t operator()(char* s) const noexcept {
		return std::hash<std::string_view>{}(s);
	}
};
struct char_ptr_equal {
	bool operator()(const char* a, const char* b) const noexcept {
		return std::strcmp(a, b) == 0;
	}
};
///////////////////////////////////////////////////////
#define FULL_WS 1<<0
void init_string(struct string*, size_t cap);
struct string get_string(size_t cap);
void free_string(struct string*);
struct string duplicate_string(struct string*);
void trim_tw(struct string*);
void clear_string(struct string*);
void dynamic_append(struct string*, char);
void dynamic_mappend(struct string*, const char*);
void dynamic_sappend(struct string*, const char*, int, int);

std::string lowercase_string(std::string);
std::string trim_string(std::string);
int randomize_number(int);
void move_iterator_past_whitespace(std::string&, int&);
std::string open_txt_file(std::filesystem::path);

#endif

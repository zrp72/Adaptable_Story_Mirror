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
#include <cctype>
#include <unordered_map>

#define debug(x) std::cout<<x

template<typename x>
using vector = std::vector<x>;

void* xmalloc(size_t bytes);
void* xcalloc(size_t amount, size_t size);
void* xrealloc(void*, size_t new_size);

//text file
struct tFILE {
	FILE* data;
	int line;
};
struct tFILE* tfopen(const char* file, const char* flags);
int tfclose(tFILE*);
int tfgetc(tFILE*);
void tungetc(int c, tFILE*);

struct zstring {
	char* text;
	size_t len;
	size_t cap;

	bool operator<(const zstring& b) const {
		return std::strcmp(this->text, b.text) < 0;
	}

	bool operator==(const zstring& b) const {
		return std::strcmp(this->text, b.text) == 0;
	}
};
struct string_hash {
	size_t operator()(const struct zstring& s) const noexcept {
		return std::hash<std::string_view>{}(s.text);
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
void init_string(struct zstring*, size_t cap);
struct zstring get_string(size_t cap);
void free_string(struct zstring*);
struct zstring duplicate_string(struct zstring*);
void trim_tw(struct zstring*);
void clear_string(struct zstring*);
void dynamic_append(struct zstring*, char);
void dynamic_mappend(struct zstring*, const char*);
void dynamic_sappend(struct zstring*, const char*, int, int);

std::string lowercase_string(std::string);
std::string trim_string(std::string);
int randomize_number(int);
void move_iterator_past_whitespace(std::string&, int&);
std::string open_txt_file(std::filesystem::path);

#endif

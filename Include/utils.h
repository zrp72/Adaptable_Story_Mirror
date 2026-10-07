#ifndef basic_functions
#define basic_functions 1
#include <string>
#include <filesystem>

std::string lowercase_string(std::string);
std::string trim_string(std::string);
int randomize_number(int);
void move_iterator_past_whitespace(std::string&, int&);
std::string open_txt_file(std::filesystem::path);

void trim_lt(char*);

#endif

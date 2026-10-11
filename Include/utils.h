#ifndef ASM_utils
#define ASM_utils 1
#include <string>
#include <filesystem>

#define debug(x) std::cout<<x

std::string lowercase_string(std::string);
std::string trim_string(std::string);
int randomize_number(int);
void move_iterator_past_whitespace(std::string&, int&);
std::string open_txt_file(std::filesystem::path);

#endif

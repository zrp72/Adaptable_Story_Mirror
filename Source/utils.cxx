#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <cstring>

#include "utils.h"

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

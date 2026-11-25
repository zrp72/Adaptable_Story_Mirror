#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "instruction_loader.h"

void
load_ral(const char* filename, struct RAL* config){
	FILE* f = fopen(filename, "r");
	if(f == NULL){
		perror(filename);
		exit(1);
	}

	int c;
	char buffer[64];
	char* d = buffer;
	char* dz = buffer;
	int rs = 1;
	while(EOF != (c = fgetc(f))){
		if((rs != 3) && isspace(c)){
			continue;
		}

		switch(rs){
		case 1:
			if(c == ':'){
				*d = 0;
				if(strncmp(buffer, "story_file", 10) == 0){
					d = config->story_file;

				} else if(strncmp(buffer, "start_chapter", 13) == 0){
					d = config->start_chapter;

				} else if(strncmp(buffer, "asset_folder", 12) == 0) {
					d = config->asset_folder;

				} else {
					fprintf(stderr, "Invalid destination in config file: '%s'\n", buffer);
					exit(1);
				}
				dz = d;
				rs = 2;
				continue;
			}
			break;
		case 2:
			if(c == '"'){
				rs = 3;
			}
			continue;
		case 3:
			if(c == '"'){
				*d = 0;
				d = buffer;
				dz = d;
				rs = 1;
				continue;
			}
			break;
		}

		if((d - dz) >= (63)){
			fprintf(stderr, "Config file has either an invalid RAL name or a filename that exceeds the buffer size.");
			exit(1);
		} else {
			*d++ = tolower((char)c);
		}
	}
	if( config->story_file[0]		== '\0'	||
		config->start_chapter[0]	== '\0'	||
		config->asset_folder[0]		== '\0'
	  ){
		fprintf(stderr, "1 or more RAL fields were left empty");
		exit(1);
	}
}


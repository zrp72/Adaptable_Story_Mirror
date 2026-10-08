#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

//#include "instruction_loader.h"

struct RAL /* Runtime Access Locations */
{
	char story_file[64];
	char start_index[64];
	char asset_folder[64];
};

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
				if(strncmp(buffer, "STORY_FILE", 10) == 0){
					d = config->story_file;

				} else if(strncmp(buffer, "START_INDEX", 11) == 0){
					d = config->start_index;

				} else if(strncmp(buffer, "ASSET_FOLDER", 12) == 0) {
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
				rs = 1;
				continue;
			}
			break;
		}

		if((d - dz) >= (63)){
			fprintf(stderr, "Config file has either an invalid RAL name or a filename that exceeds the buffer size.");
			exit(1);
		} else {
			*d++ = (char)c;
		}
	}
}

int 
main(void){
	struct RAL config;
	load_ral("READ_DIRECTIONS", &config);

	printf("%s\n%s\n%s\n", config.story_file, config.start_index, config.asset_folder);

	return 0;
}

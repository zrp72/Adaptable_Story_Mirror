#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

//#include "instruction_loader.h"

struct RAL /* Runtime Access Locations */
{
	char* story_file;
	char* start_index;
	char* asset_folder;
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
	char* d = NULL;
	int rs = 1;
	while(-1 != (c = fgetc(f))){
		if((rs != 3) && isspace(c)){
			continue;
		}

		switch(rs){
		case 1:
			if(c == ':'){
				if(strncmp(buffer, "STORY_FILE", 10) == 0){
					d = config->story_file;

				} else if(strncmp(buffer, "START_INDEX", 11) == 0){
					d = config->start_index;

				} else if(strncmp(buffer, "ASSET_FOLDER", 12) == 0) {
					d = config->asset_folder;

				} else {
					fprintf(stderr, "Config line %d is invalid.\n", buffer);
					exit(1);
				}

				memset(buffer, '\0', sizeof(buffer));
				rs = 2;
				continue;
			}
		case 2:
			if(c == '"'){
				rs = 3;
			}
			continue;
		case 3:
			if(c == '"'){
				memcpy(d, buffer, strlen(buffer));
				memset(buffer, '\0', sizeof(buffer));
				rs = 1;
				continue;
			}
		}
		if((sizeof(buffer) - strlen(buffer) - 1) == 0){
			fprintf(stderr, "Config file has either an invalid RAL name or a filename that exceeds the buffer size.");
			exit(1);
		} else {
			strcat(buffer, (char)c);
		}
	}
}


void
load_ral(const char* filename, struct RAL* config){

	FILE* f = fopen(filename, "r");
	if(f == NULL){
		perror(filename);
		exit(1);
	}

	char buffer[1024];
	int line = 1;
	char* rv;

	while(NULL != (rv = fgets(buffer, sizeof buffer, f))){

		char* colon = strchr(buffer, ':');

		if(colon == NULL){
			fprintf(stderr, "Invalid config at line %d.\n", line);
			exit(1);
		}

		char* s = buffer;

		while(isspace(*s)){
			s += 1;
		}

		char* o;
		if(strncmp(s, "STORY_FILE", 10) == 0){
			o = config->story_file;
		} else if(strncmp(s, "START_INDEX", 11) == 0){
			o = config->start_index;
		} else if(strncmp(s, "ASSET_FOLDER", 12) == 0) {
			o = config->asset_folder;
		} else {
			fprintf(stderr, "Config line %d is invalid.\n", line);
			line += 1;
			continue;
		}

		s = strchr(s, '"');
		char* e = strchr(s + 1, '"');

		if(s == NULL || e == NULL){
			printf("Invalid quotations on line %d\n", line);
		}
		*e = '\0';

		memcpy(o, s + 1, e - s);

		line += 1;
	}

	if(ferror(f)){
		fprintf(stderr, "Failed to read RD: %s\n", strerror(errno));
		exit(1);
	}
}

int main(){
	char heap[8192];
	struct RAL config = {
		.story_file = heap,
		.start_index = heap + 1024,
		.asset_folder = heap + 2048
	};
	load_ral("READ_DIRECTIONS", &config);

	printf("%s\n%s\n%s\n", config.story_file, config.start_index, config.asset_folder);

	return 0;
}

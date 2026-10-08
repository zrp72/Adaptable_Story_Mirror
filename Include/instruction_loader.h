#ifndef inst_load_header
#define inst_load_header

struct RAL /* Runtime Access Locations */
{
	char story_file[64];
	char start_index[64];
	char asset_folder[64];
};
void load_ral(const char* filename, struct RAL*);

#endif

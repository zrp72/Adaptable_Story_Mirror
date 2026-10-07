#ifndef inst_load_header
#define inst_load_header

struct RAL /* Runtime Access Locations */
{
	char[64] story_file;
	char[64] start_index;
	char[64] asset_folder;
};
void load_ral(const char* filename, struct RAL*);

#endif

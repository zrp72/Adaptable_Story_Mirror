#ifndef progress_story_header
#define progress_story_header 1

#include "story_parser.h"
#include "gui.h"
#include "utils.h"

int progress_story(struct story&, assets&, gui_core&, int choice);
bool compare_sv(int, int, int);

#endif

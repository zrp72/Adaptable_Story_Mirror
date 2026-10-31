#ifndef launch_header
#define launch_header 1

#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>

#include "utils.h"
#include "story_parser.h"
#include "gui.h"

int launch_game(struct story&, assets&, gui_core&);

#endif


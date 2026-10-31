
.PHONY: clean all

ifeq ($(OS),Windows_NT)
    HOME_DIR ?= $(USERPROFILE)
else
    HOME_DIR ?= $(HOME)
endif

CXXFLAGS = -std=c++20 -IInclude \
	-I$(HOME_DIR)\Documents\SDKs\SDL\x86_64-w64-mingw32\include \
	-I$(HOME_DIR)\Documents\SDKs\SDL_image\x86_64-w64-mingw32\include \
	-I$(HOME_DIR)\Documents\SDKs\SDL_ttf\x86_64-w64-mingw32\include

LDLIBS = -lSDL3 -lSDL3_image -lSDL3_ttf

obj_files = \
	Source/main.o \
	Source/instruction_loader.o \
	Source/story_parser.o \
	Source/gui.o \
	Source/glyphtext.o \
	Source/launch.o \
	Source/progress.o \
	Source/utils.o \

%.o: %.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++20 -c $< -o $@

ASM: $(obj_files)
	$(CXX) $(LDLIBS) $^ -o ASM

clean:
	rm -f *.o

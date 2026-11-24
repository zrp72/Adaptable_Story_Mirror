
.PHONY: clean all

obj_files = \
	Source/main.o \
	Source/instruction_loader.o \
	Source/story_parser.o \
	Source/gui.o \
	Source/glyphtext.o \
	Source/launch.o \
	Source/progress_story.o \
	Source/utils.o

CXXFLAGS = -std=c++20
CPPFLAGS = \
	-IInclude \
	-IC:\Users\purse\Documents\SDKs\SDL\x86_64-w64-mingw32\include \
	-IC:\Users\purse\Documents\SDKs\SDL_image\x86_64-w64-mingw32\include \
	-IC:\Users\purse\Documents\SDKs\SDL_ttf\x86_64-w64-mingw32\include

LDLIBS = -lSDL3 -lSDL3_image -lSDL3_ttf

%.o: %.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++20 -c $< -o $@

ASM: $(obj_files)
	$(CXX) $(CXXFLAGS) $^ -o ASM $(LDLIBS) -LInclude 

clean:
	rm -f Source/*.o ASM

image:
	docker build -t adaptable-story-mirror .

run:
	docker run adaptable-story-mirror


.PHONY: clean all

LDLIBS = -lSDL3 -lSDL3_image -lSDL3_ttf

obj_files = \
	Main.o \
	basic_functions.o \
	GUI.o \
	GUI_Assets.o \
	ParseStory.o \
	ProgressStory.o \
	UI_element_creation.o \
	glyphtext.o

all: text_rpg

%.o: %.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++20 -c $< -o $@

text_rpg: $(obj_files)
	$(CXX) $(LDLIBS) $^ -o text_rpg

clean:
	rm -f *.o

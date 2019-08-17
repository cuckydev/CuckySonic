#Release and debug
ifeq ($(RELEASE), 1)
	CXXFLAGS = -O3 -s -flto
	FILENAME_DEF = release
else
	CXXFLAGS = -O0 -g -DDEBUG
	FILENAME_DEF = debug
endif

FILENAME ?= $(FILENAME_DEF)

ifeq ($(CONSOLE), 1)
	CXXFLAGS += -mconsole
endif

#CXX flags and libraries
CXXFLAGS += `pkg-config --cflags sdl2` -MMD -MP -MF -Wall -Wextra -pedantic -Wformat-overflow=0 -static
LIBS += `pkg-config --libs sdl2 --static`

#Sources to compile
SOURCES = \
	Main \
	Error \
	Path \
	Render \
	Event \
	Input \
	Audio \
	MathUtil \
	Fade \
	Mappings \
	Level \
	LevelCollision \
	Game \
	GM_Splash \
	GM_Title \
	GM_Game \
	Player \
	Object \
	Camera \
	Objects/PathSwitcher \
	Objects/Goalpost \

#What to compile
OBJECTS = $(addprefix obj/$(FILENAME)/, $(addsuffix .o, $(SOURCES)))
DEPENDENCIES = $(addprefix obj/$(FILENAME)/, $(addsuffix .o.d, $(SOURCES)))

#Compilation code
all: build/$(FILENAME)

build/$(FILENAME): $(OBJECTS)
	@mkdir -p $(@D)
	@echo Linking...
	@$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)
	@echo Finished linking $@

obj/$(FILENAME)/%.o: src/%.cpp
	@mkdir -p $(@D)
	@echo Compiling $<
	@$(CXX) $(CXXFLAGS) $< -o $@ -c
	@echo Compiled!
	
include $(wildcards $(DEPENDENCIES))

#Remove all our compiled objects
clean:
	@rm -rf build obj

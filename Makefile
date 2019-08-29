#Release and debug
ifeq ($(RELEASE), 1)
	CXXFLAGS = -O3 -s -flto
	FILENAME_DEF = release
else
	CXXFLAGS = -O0 -ggdb3 -DDEBUG
	FILENAME_DEF = debug
endif

FILENAME ?= $(FILENAME_DEF)

ifeq ($(CONSOLE), 1)
	CXXFLAGS += -mconsole
endif

#CXX flags and libraries
CXXFLAGS += `pkg-config --cflags sdl2` -MMD -MP -MF -Wall -Wextra -pedantic -Wformat-overflow=0
LIBS += `pkg-config --libs sdl2 --static` -static

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
	TitleCard \
	LevelSpecific/GHZ \
	LevelSpecific/EHZ \
	Objects/PathSwitcher \
	Objects/Goalpost \
	Objects/Spiral \
	Objects/Bridge \
	Objects/Motobug \

#What to compile
OBJECTS = $(addprefix obj/$(FILENAME)/, $(addsuffix .o, $(SOURCES)))
DEPENDENCIES = $(addprefix obj/$(FILENAME)/, $(addsuffix .o.d, $(SOURCES)))

#If compiling a windows build, add the Windows icon object into our executable
ifeq ($(WINDOWS), 1)
	OBJECTS += obj/$(FILENAME)/WindowsIcon.o
endif

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

#Compile the Windows icon file into an object
obj/$(FILENAME)/WindowsIcon.o: res/icon.rc res/icon.ico
	@mkdir -p $(@D)
	@windres $< $@

#Remove all our compiled objects
clean:
	@rm -rf build obj

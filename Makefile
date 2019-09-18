#Release and debug
ifeq ($(RELEASE), 1)
	CXXFLAGS = -O3 -flto
	LDFLAGS = -s
	FILENAME_DEF = release
else
	CXXFLAGS = -O0 -ggdb3 -DDEBUG
	FILENAME_DEF = debug
endif

FILENAME ?= $(FILENAME_DEF)

ifeq ($(WINDOWS), 1)
	CXXFLAGS += -DWINDOWS
	
	ifeq ($(CONSOLE), 1)
		CXXFLAGS += -mconsole
	endif
endif

#CXX flags and libraries
CXXFLAGS += `pkg-config --cflags sdl2` -MMD -MP -MF $@.d 

ifeq ($(STATIC), 1)
	LDFLAGS += -static
	LIBS += `pkg-config --libs sdl2 --static`
else
	LIBS += `pkg-config --libs sdl2`
endif

#Sources to compile
SOURCES = \
	Main \
	Error \
	Path \
	Render \
	Render_Blit \
	Event \
	Input \
	Audio \
	Audio_stb_vorbis \
	Audio_miniaudio \
	MathUtil \
	Fade \
	Mappings \
	Level \
	LevelCollision \
	BackgroundScroll \
	Game \
	GM_Splash \
	GM_Title \
	GM_Game \
	GM_SpecialStage \
	Player \
	Object \
	Camera \
	TitleCard \
	Hud \
	LevelSpecific/GHZ \
	LevelSpecific/EHZ \
	Objects/PathSwitcher \
	Objects/Goalpost \
	Objects/Spiral \
	Objects/Bridge \
	Objects/Sonic1Scenery \
	Objects/Ring \
	Objects/BouncingRing \
	Objects/Explosion \
	Objects/Motobug \
	Objects/Chopper \
	Objects/GHZPlatform \
	Objects/GHZEdgeWall \
	Objects/SwingingPlatform \
	Objects/PurpleRock \
	Objects/Monitor \

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
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)
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

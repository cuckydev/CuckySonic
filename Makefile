#Release and debug
ifeq ($(RELEASE), 1)
	CXXFLAGS = -O3 -flto -Wall -Wextra
	LDFLAGS = -s
	FILENAME_DEF = release
else
	CXXFLAGS = -Og -ggdb3 -Wall -Wextra -pedantic -DDEBUG
	FILENAME_DEF = debug
endif

FILENAME ?= $(FILENAME_DEF)

ifeq ($(OS), Windows_NT)
	WINDOWS ?= 1
endif

ifeq ($(WINDOWS), 1)
	CXXFLAGS += -DWINDOWS
	
	ifneq ($(RELEASE), 1)
		CXXFLAGS += -mconsole
	endif
endif

#Use SDL2 as the default backend if one wasn't explicitly chosen
BACKEND ?= SDL2

#Backend flags and libraries
ifeq ($(BACKEND), SDL2)
	CXXFLAGS += `pkg-config --cflags sdl2` -DBACKEND_SDL2

	ifeq ($(STATIC), 1)
		LDFLAGS += -static
		LIBS += `pkg-config --libs sdl2 --static`
	else
		LIBS += `pkg-config --libs sdl2`
	endif
endif

#Other CXX flags
CXXFLAGS += -faligned-new -MMD -MP -MF $@.d

#Sources to compile
SOURCES = \
	Main \
	Audio_stb_vorbis \
	Audio_miniaudio \
	MathUtil \
	Fade \
	Mappings \
	Level \
	LevelCollision \
	SpecialStage \
	Background \
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
	Objects/AttractRing \
	Objects/Explosion \
	Objects/Motobug \
	Objects/Chopper \
	Objects/GHZPlatform \
	Objects/GHZEdgeWall \
	Objects/SwingingPlatform \
	Objects/PurpleRock \
	Objects/Monitor \
	Objects/Spring \
	Objects/Minecart

#Backend source files
ifeq ($(BACKEND), SDL2)
	SOURCES += \
		Backend_SDL2/Error \
		Backend_SDL2/Filesystem \
		Backend_SDL2/Render \
		Backend_SDL2/Render_Blit \
		Backend_SDL2/Event \
		Backend_SDL2/Input \
		Backend_SDL2/Audio
endif

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
	
include $(wildcard $(DEPENDENCIES))

#Compile the Windows icon file into an object
obj/$(FILENAME)/WindowsIcon.o: res/icon.rc res/icon.ico
	@mkdir -p $(@D)
	@windres $< $@

#Remove all our compiled objects
clean:
	@rm -rf obj

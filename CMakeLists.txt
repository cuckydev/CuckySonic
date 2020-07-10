cmake_minimum_required(VERSION 3.7.2)

if((${CMAKE_VERSION} VERSION_EQUAL 3.9) OR (${CMAKE_VERSION} VERSION_GREATER 3.9))
	cmake_policy(SET CMP0069 NEW)
endif()

set(BUILD_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

option(FORCE_LOCAL_LIBS "Compile the built-in versions of SDL2 instead of using the system-provided ones" OFF)
set(BACKEND "SDL2" CACHE STRING "Which backend system to use: currently the only options is 'SDL2'")

project(CuckySonic LANGUAGES C CXX)

if(MSVC)
	# Statically-link the CRT (vcpkg static libs do this)
	foreach(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif()
	endforeach()
endif()

add_executable(CuckySonic
	src/Audio.h
	src/Audio_miniaudio.cpp
	src/Audio_miniaudio.h
	src/Audio_stb_vorbis.cpp
	src/Background.cpp
	src/Background.h
	src/Camera.cpp
	src/Camera.h
	src/CommonMacros.h
	src/Endianness.h
	src/Error.h
	src/Event.h
	src/Fade.cpp
	src/Fade.h
	src/Filesystem.h
	src/Game.cpp
	src/Game.h
	src/GameConstants.h
	src/GM.h
	src/GM_Game.cpp
	src/GM_SpecialStage.cpp
	src/GM_Splash.cpp
	src/GM_Title.cpp
	src/Hud.cpp
	src/Hud.h
	src/Input.h
	src/Level.cpp
	src/Level.h
	src/LevelCollision.cpp
	src/LevelCollision.h
	src/LevelSpecific.h
	src/Log.h
	src/Main.cpp
	src/Mappings.cpp
	src/Mappings.h
	src/MathUtil.cpp
	src/MathUtil.h
	src/Object.cpp
	src/Object.h
	src/Objects.h
	src/Player.cpp
	src/Player.h
	src/Render.h
	src/SpecialStage.cpp
	src/SpecialStage.h
	src/TitleCard.cpp
	src/TitleCard.h
	src/LevelSpecific/GHZ.cpp
	src/LevelSpecific/EHZ.cpp
	src/Objects/PathSwitcher.cpp
	src/Objects/Goalpost.cpp
	src/Objects/Spiral.cpp
	src/Objects/Bridge.cpp
	src/Objects/Sonic1Scenery.cpp
	src/Objects/Ring.cpp
	src/Objects/BouncingRing.cpp
	src/Objects/AttractRing.cpp
	src/Objects/Explosion.cpp
	src/Objects/Motobug.cpp
	src/Objects/Chopper.cpp
	src/Objects/GHZPlatform.cpp
	src/Objects/GHZEdgeWall.cpp
	src/Objects/SwingingPlatform.cpp
	src/Objects/PurpleRock.cpp
	src/Objects/Monitor.cpp
	src/Objects/Spring.cpp
	src/Objects/Minecart.cpp
)

# Enable the SDL2 backend if it's selected
if(BACKEND MATCHES "SDL2")
	target_sources(CuckySonic PRIVATE
		src/Backend_SDL2/Audio.cpp
		src/Backend_SDL2/Error.cpp
		src/Backend_SDL2/Event.cpp
		src/Backend_SDL2/Filesystem.cpp
		src/Backend_SDL2/Input.cpp
		src/Backend_SDL2/Render.cpp
		src/Backend_SDL2/Render_Blit.cpp
	)

	target_compile_definitions(CuckySonic PRIVATE BACKEND_SDL2)
else()
	message(FATAL_ERROR "Invalid BACKEND selected")
endif()

# Make some tweaks if we're targetting Windows
if(WIN32)
	target_sources(CuckySonic PRIVATE "res/icon.rc")
	set_target_properties(CuckySonic PROPERTIES WIN32_EXECUTABLE YES)	# Disable the console window
endif()

# Make some tweaks if we're using MSVC
if(MSVC)
	target_compile_definitions(CuckySonic PRIVATE _CRT_SECURE_NO_WARNINGS)	# Disable warnings that normally fire up on MSVC when using "unsafe" functions instead of using MSVC's "safe" _s functions
endif()

# Give each type of build its own distinct name
set_target_properties(CuckySonic PROPERTIES DEBUG_OUTPUT_NAME "debug")
set_target_properties(CuckySonic PROPERTIES RELEASE_OUTPUT_NAME "release")
set_target_properties(CuckySonic PROPERTIES RELWITHDEBINFO_OUTPUT_NAME "relwithdebinfo")
set_target_properties(CuckySonic PROPERTIES MINSIZEREL_OUTPUT_NAME "minsizerel")

# Send executable to the build directory
set_target_properties(CuckySonic PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIRECTORY}
)

# Enable link-time optimisation if available
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	if((${CMAKE_VERSION} VERSION_EQUAL 3.9) OR (${CMAKE_VERSION} VERSION_GREATER 3.9))
		include(CheckIPOSupported)
		check_ipo_supported(RESULT result)
		if(result)
			set_target_properties(CuckySonic PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
		endif()
	endif()
endif()

# Find dependencies

# The SDL2 backend needs SDL2, obviously
if(BACKEND MATCHES "SDL2")
	if(NOT FORCE_LOCAL_LIBS)
		find_package(SDL2)
	endif()

	if(TARGET SDL2::SDL2)
		# CMake-generated config (Arch, vcpkg, Raspbian)
		message(STATUS "Using system SDL2")
		target_link_libraries(CuckySonic SDL2::SDL2 SDL2::SDL2main)
	elseif(TARGET SDL2::SDL2-static)
		# CMake-generated config (Arch, vcpkg, Raspbian)
		message(STATUS "Using system SDL2")
		target_link_libraries(CuckySonic SDL2::SDL2-static SDL2::SDL2main)
	elseif(SDL2_FOUND)
		# Autotools-generated config (MSYS2)
		message(STATUS "Using system SDL2")
		target_include_directories(CuckySonic PRIVATE ${SDL2_INCLUDE_DIRS})
		target_link_libraries(CuckySonic ${SDL2_LIBRARIES})
	else()
		# Compile it ourselves
		message(STATUS "Using local SDL2")
		set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
		if(MSVC)
			set(LIBC ON)	# Needed to prevent possible 'symbol already defined' errors
		endif()
		add_subdirectory("SDL2" EXCLUDE_FROM_ALL)
		target_link_libraries(CuckySonic SDL2-static SDL2main)
	endif()
endif()

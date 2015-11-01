project(crystalpicnic)
cmake_minimum_required(VERSION 2.6)

# Options

# Extras
option(EDITOR "Build battle level editor" off)
option(TOOLS "Build extra tools" off)

# Features
option(DEMO "Build the demo version" off)
option(WELL512 "Use well512 RNG" off)

# Platforms
option(RASPBERRYPI "Build for Raspberry Pi" off)

# Compile options
option(DEBUG "Build with debug symbols" off)
option(PROFILE "Build for profiling" off)
option(STABS "Use -gstabs debug symbols" off)
option(ALLEGRO_MONOLITH "Use the monolith allegro library" on)
option(STANDARD_LINUX_BUILD "Use default libraries on Linux" on)
OPTION(STEAMWORKS "Enable Steamworks" off)
if(WIN32)
	option(GNU "Use GNU tools on Windows" off)
else()
	option(GNU "Use GNU tools on Windows" on)
endif()

# Option configuration

set(ALLEGRO_SUFFIX "-static" CACHE STRING "Allegro library suffix")

if(WELL512)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DWELL512=1")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DWELL512=1")
endif()

if(RASPBERRYPI)
	set(CMAKE_C_FLAGS_RELEASE "-Wall -O3")
	set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3")
elseif(GNU)
	set(CMAKE_C_FLAGS_RELEASE "-Wall -O2")
	set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2")
endif()

if(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
	include_directories(${USER_INCLUDE_PATH})
endif(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
if(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))
	link_directories(${USER_LIBRARY_PATH})
endif(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))

include_directories("${CMAKE_SOURCE_DIR}/include")

set(COMMON_SRCS
	src/abilities.cpp
	src/animation.cpp
	src/animation_set.cpp
	src/area_loop.cpp
	src/area_manager.cpp
	src/astar.cpp
	src/astar_character_role.cpp
	src/audio_config_loop.cpp
	src/bass.cpp
	src/bass_fileprocs.cpp
	src/battle_ai.cpp
	src/battle_entity.cpp
	src/battle_loop.cpp
	src/battle_pathfind.cpp
	src/battle_transition_in.cpp
	src/bitmap.cpp
	src/bones.cpp
	src/camera.cpp
	src/character_map_entity.cpp
	src/character_role.cpp
	src/collidable.cpp
	src/collision_detection.cpp
	src/config.cpp
	src/cpa.cpp
	src/credits_loop.cpp
	src/crystal_loop.cpp
	src/difficulty_loop.cpp
	src/direct3d.cpp
	src/enemy_avatar.cpp
	src/enemy_avatar_wander_character_role.cpp
	src/engine.cpp
	src/entity.cpp
	src/error.cpp
	src/follow_character_role.cpp
	src/frame.cpp
	src/game_specific_globals.cpp
	src/general.cpp
	src/graphics.cpp
	src/input_config_loop.cpp
	src/language_config_loop.cpp
	src/lua.cpp
	src/main.cpp
	src/main_menu_loop.cpp
	src/map_entity.cpp
	src/map_loop.cpp
	src/mt19937ar.c
	src/music.cpp
	src/npc.cpp
	src/particle.cpp
	src/player.cpp
	src/resource_manager.cpp
	src/runner_loop.cpp
	src/saveload_loop.cpp
	src/settings_loop.cpp
	src/shaders.cpp
	src/shop_loop.cpp
	src/skeleton.cpp
	src/snprintf.c
	src/sound.cpp
	src/speech_loop.cpp
	src/steering.cpp
	src/tls.cpp
	src/triangulate.cpp
	src/video_config_loop.cpp
	src/video_player.cpp
	src/wander_character_role.cpp
	src/weaponized_entity.cpp
	src/well512.c
	src/whack_a_skunk_loop.cpp
	src/widgets.cpp
	src/xml.cpp
)

if(STEAMWORKS)
	LIST(APPEND COMMON_SRCS src/steamworks.cpp)
endif()

if(WIN32)
	if(GNU)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -static-libgcc -static-libstdc++ -Wno-unknown-pragmas -Wno-comment -Wno-strict-aliasing")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static-libgcc -static-libstdc++ -fcheck-new -Wno-unknown-pragmas -Wno-comment -Wno-strict-aliasing")
		if(DEMO)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DDEMO=1")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEMO=1")
		endif()
		if(DEBUG)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
		elseif(PROFILE)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pg")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pg")
		elseif(STABS)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -gstabs")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -gstabs")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -gstabs")
		endif()
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
	else()
		SET(FLAGS "/D_USE_MATH_DEFINES /DALLEGRO_STATICLINK /EHsc /D_WIN32_WINNT=0x501")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${FLAGS} /MT /MP")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${FLAGS} /MT /MP")
		set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${FLAGS} /MTd /MP")
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${FLAGS} /MTd /MP")
		if(DEMO)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /DDEMO")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DDEMO")
			set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /DDEMO")
			set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /DDEMO")
		endif()
		if(STEAMWORKS)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /DSTEAMWORKS=1")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DSTEAMWORKS=1")
			set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /DSTEAMWORKS=1")
			set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /DSTEAMWORKS=1")
		endif()
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /SUBSYSTEM:WINDOWS")
	endif()
elseif(APPLE)
	list(APPEND COMMON_SRCS src/apple.m)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-unknown-pragmas -Wno-comment")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unknown-pragmas -Wno-comment")
	if(DEMO)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DDEMO=1")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEMO=1")
	endif()
endif()

add_library(my_load_bitmap STATIC src/my_load_bitmap.cpp)
add_library(crystalpicnic-engine STATIC	${COMMON_SRCS})

if(DEMO)
	add_executable(CrystalPicnicDemo src/crystalpicnic.cpp)
else()
	add_executable(CrystalPicnic src/crystalpicnic.cpp)
endif()

if(APPLE)
	set(ALLEGRO_MAIN_LIB "allegro_main${ALLEGRO_SUFFIX}")
	FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
	FIND_LIBRARY(FOUNDATION_LIBRARY Foundation)
	FIND_LIBRARY(APPSERVICES_LIBRARY ApplicationServices)
	FIND_LIBRARY(APPKIT_LIBRARY AppKit)
	FIND_LIBRARY(IOKIT_LIBRARY IOKit)
	FIND_LIBRARY(AUDIOTOOLBOX_LIBRARY AudioToolbox)
	set(EXTRA_LIBS ${IOKIT_LIBRARY} ${OPENGL_LIBRARY} ${FOUNDATION_LIBRARY} ${APPSERVICES_LIBRARY} /Users/trent/code/local/lib/libfreetype.a ${APPKIT_LIBRARY} ${AUDIOTOOLBOX_LIBRARY} bass bassmidi atlas tgui2 wrap /Users/trent/code/local/lib/libz.a /Users/trent/code/local/lib/libpoly2tri.a)
	set(MAIN_LIBS
		crystalpicnic-engine
		wrap
		my_load_bitmap
		atlas
		allegro_monolith${ALLEGRO_SUFFIX}
		/Users/trent/code/local/lib/liblua5.2.a
		${EXTRA_LIBS}
	)
elseif(WIN32)
	set(ALLEGRO_MAIN_LIB "")
	set(EXTRA_LIBS "opengl32" "dxguid" "dinput8" "xinput" "dsound" "d3d9" "kernel32" "user32" "gdi32" "comdlg32" "ole32" "winmm" "psapi" "gdiplus" "uuid" "shlwapi")
	if(STEAMWORKS)
		LIST(APPEND EXTRA_LIBS "steam_api")
	endif()
	if(GNU)
		set(ICON_OBJ "${CMAKE_SOURCE_DIR}/windows_icon.res")
		set(FREETYPE_LIB_NAME "freetype")
		set(ZLIB_LIB_NAME "z")
	else()
		set(ICON_OBJ "c:\\Users\\trent\\code\\crystal-picnic\\windows_icon.lib")
		set(FREETYPE_LIB_NAME "freetype_static")
		set(ZLIB_LIB_NAME "zlibstatic")
		LIST(APPEND EXTRA_LIBS "libpng16_static" "jpeg-static")
	endif()
	if(DEBUG)
		set(ALLEGRO_LIB "allegro_monolith-debug")
		set(WRAP_LIB_NAME "wrap-debug")
		set(ATLAS_LIB_NAME "atlas-debug")
		set(TGUI2_LIB_NAME "tgui2-debug")
		set(POLY2TRI_LIB_NAME "poly2tri-debug")
	else()
		set(ALLEGRO_LIB "allegro_monolith-static")
		set(WRAP_LIB_NAME "wrap")
		set(ATLAS_LIB_NAME "atlas")
		set(TGUI2_LIB_NAME "tgui2")
		set(POLY2TRI_LIB_NAME "poly2tri")
	endif()
	set(LUA_LIB "lua5.2")
	set(MAIN_LIBS
		crystalpicnic-engine
		${WRAP_LIB_NAME}
		my_load_bitmap
		${ATLAS_LIB_NAME}
		${TGUI2_LIB_NAME}
		${ALLEGRO_LIB}
		${LUA_LIB}
		${FREETYPE_LIB_NAME}
		${ZLIB_LIB_NAME}
		bassmidi
		bass
		${POLY2TRI_LIB_NAME}
		${EXTRA_LIBS}
		"c:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3dx9.lib"
		${ICON_OBJ}
	)
else()
	# Linux

	if(DEMO)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DDEMO=1")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEMO=1")
	endif()

	if(RASPBERRYPI)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unknown-pragmas -march=armv6 -mfpu=vfp -mfloat-abi=hard")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unknown-pragmas -march=armv6 -mfpu=vfp -mfloat-abi=hard")

		include_directories("/opt/vc/include")
		include_directories("/opt/vc/include/interface/vcos/pthreads")

		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,stuff")
		set(MAIN_LIBS
			crystalpicnic-engine
			wrap
			my_load_bitmap
			atlas
			tgui2
			allegro_monolith${ALLEGRO_SUFFIX}
			${ALLEGRO_MAIN_LIB}
			/home/trent/code/crystal-picnic/build/libbassmidi.so
			/home/trent/code/crystal-picnic/build/libbass.so
			lua5.2
			freetype
			png
			z
			poly2tri
			GLESv2
			EGL
			bcm_host
			X11
			pthread
		)
	else(RASPBERRYPI)
		# Desktop Linux

		if(DEBUG)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g -Wall -Wno-unknown-pragmas -Wno-comment")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -Wall -Wno-unknown-pragmas -Wno-comment")
		else()
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -Wall -Wno-unknown-pragmas -Wno-comment")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall -Wno-unknown-pragmas -Wno-comment")
		endif()

		if(ALLEGRO_MONOLITH)
			set(ALLEGRO_LIBRARIES allegro_monolith${ALLEGRO_SUFFIX} ${ALLEGRO_MAIN_LIB})
		else()
			set(ALLEGRO_LIBRARIES
				allegro${ALLEGRO_SUFFIX}
				allegro_font${ALLEGRO_SUFFIX}
				allegro_image${ALLEGRO_SUFFIX}
				allegro_memfile${ALLEGRO_SUFFIX}
				allegro_primitives${ALLEGRO_SUFFIX}
				allegro_ttf${ALLEGRO_SUFFIX}
			)
		endif()

		if(STANDARD_LINUX_BUILD)
			set(LUA_LIBRARY lua5.2)
			set(PNG_LIBRARY png)
			set(FREETYPE_LIBRARY freetype)
			set(ZLIB_LIBRARY z)
			set(BASS_LIBRARY bass)
			set(BASSMIDI_LIBRARY bassmidi)
			set(POLY2TRI_LIBRARY poly2tri)
			set(OTHER_LIBRARIES pthread)
		else()
			set(LUA_LIBRARY "/usr/local/lib/liblua5.2.a")
			set(PNG_LIBRARY "/usr/local/lib/libpng.a")
			set(FREETYPE_LIBRARY "/usr/local/lib/libfreetype.a")
			set(ZLIB_LIBRARY "/usr/local/lib/libz.a")
			set(BASS_LIBRARY "/home/trent/code/crystal-picnic/build/libbass.so")
			set(BASSMIDI_LIBRARY "/home/trent/code/crystal-picnic/build/libbassmidi.so")
			set(POLY2TRI_LIBRARY "/usr/local/lib/libpoly2tri.a")
			set(OTHER_LIBRARIES "")
		endif()

		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,\$ORIGIN/stuff")

		set(MAIN_LIBS
			crystalpicnic-engine
			atlas
			wrap
			tgui2
			my_load_bitmap
			${ALLEGRO_LIBRARIES}
			Xcursor Xi Xinerama Xrandr GLU GL SM ICE X11 Xext
			${LUA_LIBRARY}
			${PNG_LIBRARY}
			${FREETYPE_LIBRARY}
			${ZLIB_LIBRARY}
			${BASS_LIBRARY}
			${BASSMIDI_LIBRARY}
			${POLY2TRI_LIBRARY}
			${OTHER_LIBRARIES}
		)
	endif()
endif()

if(DEMO)
	target_link_libraries(CrystalPicnicDemo ${MAIN_LIBS})
else()
	target_link_libraries(CrystalPicnic ${MAIN_LIBS})
endif()

# -- Tools --

if(EDITOR)
   add_subdirectory(tools/battled2)
endif(EDITOR)

if(TOOLS)
   add_subdirectory(tools)
endif(TOOLS)

set(CMAKE_LEGACY_CYGWIN_WIN32 0)
project(crystalpicnic)
cmake_minimum_required(VERSION 2.6)
#cmake_policy(SET CMP0015 OLD)

# Options

option(EDITOR "Build battle level editor" off)
option(TOOLS "Build extra tools" off)
option(WELL512 "Use well512 RNG" off)
option(RASPBERRYPI "Build for Raspberry Pi" off)
if(WIN32)
	option(GNU "Use GNU tools on Windows" off)
else()
	option(GNU "Use GNU tools on Windows" on)
endif()
option(DEMO "Build the demo version" off)
option(DEBUG "Build with debug symbols" off)
option(PROFILE "Build for profiling" off)
option(STABS "Use -gstabs" off)
option(LUAJIT "Use LuaJIT" off)

# Option configuration

set(ALLEG_SUFFIX "-static" CACHE STRING "Allegro library suffix")

if(WELL512)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DWELL512=1")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DWELL512=1")
endif()

include_directories("${CMAKE_SOURCE_DIR}/include")

if(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
	include_directories(${USER_INCLUDE_PATH})
endif(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
if(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))
	link_directories(${USER_LIBRARY_PATH})
endif(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))

if(GNU)
	set(CMAKE_C_FLAGS_RELEASE "-Wall")
	set(CMAKE_CXX_FLAGS_RELEASE "-Wall")
	set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG=1")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG=1")
endif(GNU)

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
		else()
			if(PROFILE)
				set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pg")
				set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pg")
			else()
				if(STABS)
					set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -gstabs")
					set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -gstabs")
					set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -gstabs")
				endif()
			endif()
		endif()
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
	else()
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /D_USE_MATH_DEFINES /DALLEGRO_STATICLINK /EHsc /D_WIN32_WINNT=0x501 /MT")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /D_USE_MATH_DEFINES /DALLEGRO_STATICLINK /EHsc /D_WIN32_WINNT=0x501 /MT")
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_USE_MATH_DEFINES /DALLEGRO_STATICLINK /EHsc /D_WIN32_WINNT=0x501 /MTd")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_USE_MATH_DEFINES /DALLEGRO_STATICLINK /EHsc /D_WIN32_WINNT=0x501 /MTd")
		if(DEMO)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /DDEMO")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DDEMO")
			set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /DDEMO")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEMO")
		endif()
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /SUBSYSTEM:WINDOWS")
	endif()
endif()

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

if(APPLE)
	list(APPEND COMMON_SRCS src/apple.m)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-unknown-pragmas -Wno-comment")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unknown-pragmas -Wno-comment")
	if(DEMO)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DDEMO=1")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEMO=1")
	endif()
endif()

add_library(my_load_bitmap
	STATIC
	src/my_load_bitmap.cpp
)

add_library(crystalpicnic-engine
	STATIC
	${COMMON_SRCS}
)

if(DEMO)
	add_executable(CrystalPicnicDemo src/crystalpicnic.cpp)
else()
	add_executable(CrystalPicnic src/crystalpicnic.cpp)
endif()

if(APPLE)
	set(ALLEGRO_MAIN_LIB "allegro_main${ALLEG_SUFFIX}")
	FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
	FIND_LIBRARY(FOUNDATION_LIBRARY Foundation)
	FIND_LIBRARY(APPSERVICES_LIBRARY ApplicationServices)
	FIND_LIBRARY(APPKIT_LIBRARY AppKit)
	FIND_LIBRARY(IOKIT_LIBRARY IOKit)
	FIND_LIBRARY(AUDIOTOOLBOX_LIBRARY AudioToolbox)
	SET(EXTRA_LIBS ${IOKIT_LIBRARY} ${OPENGL_LIBRARY} ${FOUNDATION_LIBRARY} ${APPSERVICES_LIBRARY} /Users/trent/code/local/lib/libfreetype.a ${APPKIT_LIBRARY} ${AUDIOTOOLBOX_LIBRARY} bass bassmidi atlas tgui2 wrap /Users/trent/code/local/lib/libz.a /Users/trent/code/local/lib/libpoly2tri.a)
endif(APPLE)

if(WIN32)
	set(ALLEGRO_MAIN_LIB "")
	set(EXTRA_LIBS "opengl32" "dxguid" "dinput8" "xinput" "dsound" "d3d9" "kernel32" "user32" "gdi32" "comdlg32" "ole32" "winmm" "psapi" "gdiplus" "uuid" "shlwapi" "wsock32" "ws2_32" )
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
	if(LUAJIT)
		set(LUA_LIB "c:\\Users\\trent\\code\\crystalpicnic\\build\\lua51.dll")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DLUAJIT=1 -DLUA_BUILD_AS_DLL=1")
	else()
		set(LUA_LIB "lua5.2")
	endif()
	if(DEBUG)
		SET(ALLEGRO_LIB "allegro_monolith-debug")
		SET(WRAP_LIB_NAME "wrap-debug")
		SET(ATLAS_LIB_NAME "atlas-debug")
		SET(TGUI2_LIB_NAME "tgui2-debug")
		SET(POLY2TRI_LIB_NAME "poly2tri-debug")
	else()
		SET(ALLEGRO_LIB "allegro_monolith-static")
		SET(WRAP_LIB_NAME "wrap")
		SET(ATLAS_LIB_NAME "atlas")
		SET(TGUI2_LIB_NAME "tgui2")
		SET(POLY2TRI_LIB_NAME "poly2tri")
	endif()
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
else(WIN32)
	if(APPLE)
		set(MAIN_LIBS
			crystalpicnic-engine
			wrap
			my_load_bitmap
			atlas
			allegro_ttf${ALLEG_SUFFIX}
			allegro_font${ALLEG_SUFFIX}
			allegro_primitives${ALLEG_SUFFIX}
			allegro_memfile${ALLEG_SUFFIX}
			allegro_dialog${ALLEG_SUFFIX}
			allegro_image${ALLEG_SUFFIX}
			allegro${ALLEG_SUFFIX} ${ALLEGRO_MAIN_LIB}
			/Users/trent/code/local/lib/liblua.a
			${EXTRA_LIBS}
		)
	else(APPLE)
		# Linux/Raspberry Pi
		if(DEMO)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DDEMO=1")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEMO=1")
		endif()

		if(RASPBERRYPI)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unknown-pragmas -march=armv6 -mfpu=vfp -mfloat-abi=hard")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unknown-pragmas -march=armv6 -mfpu=vfp -mfloat-abi=hard")
			include_directories("/opt/vc/include")
			include_directories("/opt/vc/include/interface/vcos/pthreads")

			if(LUAJIT)
				set(LUA_LIB "/Users/trent/code/crystalpicnic/build-raspberrypi/libluajit.so")
				set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DLUAJIT=1")
			else()
				set(LUA_LIB "lua5.2")
			endif()

			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,\$ORIGIN/stuff")
			set(MAIN_LIBS
				crystalpicnic-engine
				wrap
				my_load_bitmap
				atlas
				allegro_ttf${ALLEG_SUFFIX}
				allegro_font${ALLEG_SUFFIX}
				allegro_primitives${ALLEG_SUFFIX}
				allegro_memfile${ALLEG_SUFFIX}
				allegro_image${ALLEG_SUFFIX}
				allegro${ALLEG_SUFFIX}
				${ALLEGRO_MAIN_LIB}
				SM
				ICE
				X11
				Xext
				Xcursor
				Xinerama
				Xrandr
				Xpm
				xcb
				Xau
				Xdmcp
				bcm_host GLESv2 EGL m pthread
				/Users/trent/code/crystalpicnic/build-raspberrypi/libbassmidi.so
				/Users/trent/code/crystalpicnic/build-raspberrypi/libbass.so
				${LUA_LIB}
				png
				freetype
				z
				atlas
				tgui2
				wrap
				poly2tri
				rt
			)
		else(RASPBERRYPI)
			if(DEBUG)
				set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g -Wall -Wno-unknown-pragmas -Wno-comment")
				set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -Wall -Wno-unknown-pragmas -Wno-comment")
			else()
				set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -Wall -Wno-unknown-pragmas -Wno-comment")
				set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall -Wno-unknown-pragmas -Wno-comment")
			endif()
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,\$ORIGIN/stuff")
			set(MAIN_LIBS
				crystalpicnic-engine
				atlas
				wrap
				tgui2
				my_load_bitmap
				allegro_ttf${ALLEG_SUFFIX}
				allegro_font${ALLEG_SUFFIX}
				allegro_primitives${ALLEG_SUFFIX}
				allegro_memfile${ALLEG_SUFFIX}
				allegro_image${ALLEG_SUFFIX}
				allegro${ALLEG_SUFFIX}
				${ALLEGRO_MAIN_LIB}
				Xcursor Xi Xinerama Xrandr GLU GL SM ICE X11 Xext
				/usr/local/lib/liblua5.2.a
				/usr/local/lib/libpng.a
				/usr/local/lib/libfreetype.a
				/usr/local/lib/libz.a
				/home/trent/code/crystalpicnic/build/libbass.so
				/home/trent/code/crystalpicnic/build/libbassmidi.so
				/usr/local/lib/libpoly2tri.a
			)
		endif()
	endif(APPLE)
endif(WIN32)

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


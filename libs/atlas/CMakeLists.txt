cmake_minimum_required(VERSION 2.6)
project(atlas CXX)

option(WANT_EXAMPLE "Build the example" off)
option(DEBUG "Enable debug build" off)

set(ALLEG_SUFFIX "-static")

if(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
	include_directories(${USER_INCLUDE_PATH})
endif(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
if(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))
	link_directories(${USER_LIBRARY_PATH})
endif(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))

include_directories("include")

set(ATLAS_SRC src/atlas.cpp src/atlas_accessor.cpp)
set(EX_ATLAS_SRC src/ex_atlas.cpp)
add_library(atlas STATIC ${ATLAS_SRC})

if(WIN32)
	set_target_properties(atlas PROPERTIES COMPILE_FLAGS "/MT")
endif()

if(WANT_EXAMPLE)
	add_executable(ex_atlas ${EX_ATLAS_SRC})

	if(APPLE)
		FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
		FIND_LIBRARY(FOUNDATION_LIBRARY Foundation)
		FIND_LIBRARY(APPSERVICES_LIBRARY ApplicationServices)
		FIND_LIBRARY(APPKIT_LIBRARY AppKit)
		FIND_LIBRARY(IOKIT_LIBRARY IOKit)
		MARK_AS_ADVANCED(OPENGL_LIBRARY)
		MARK_AS_ADVANCED(FOUNDATION_LIBRARY)
		MARK_AS_ADVANCED(APPSERVICES_LIBRARY)
		MARK_AS_ADVANCED(IOKIT_LIBRARY)
		SET(EXTRA_LIBS ${IOKIT_LIBRARY} ${OPENGL_LIBRARY} ${FOUNDATION_LIBRARY} ${APPSERVICES_LIBRARY} ${APPKIT_LIBRARY})
		SET(CMAKE_CXX_COMPILER "g++")
		set(CMAKE_CXX_FLAGS "-stdlib=libstdc++")
	else(APPLE)
		if(WIN32)
			SET(EXTRA_LIBS "opengl32" "dxguid" "dinput8" "dsound" "d3d9" "kernel32" "user32" "gdi32" "comdlg32" "ole32" "winmm" "psapi" "gdiplus" "uuid" "shlwapi")
			set_target_properties(ex_atlas PROPERTIES COMPILE_FLAGS "/MT")
			set_target_properties(ex_atlas PROPERTIES LINK_FLAGS "/LINK setargv.obj")
		else()
			SET(EXTRA_LIBS "Xcursor" "Xi" "Xinerama" "Xrandr" "GLU" "GL" "SM" "ICE" "X11" "Xext" "png" "z")
		endif()
	endif(APPLE)

	# FIXME: add Linux/X
		
	target_link_libraries(
		ex_atlas
		atlas
		wrap
		allegro_monolith${ALLEG_SUFFIX}
		${EXTRA_LIBS}
	)
endif()

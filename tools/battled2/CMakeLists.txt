cmake_minimum_required(VERSION 2.6)
project(battled2)

# Options

set(ALLEG_SUFFIX "-static")

if(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
	include_directories(${USER_INCLUDE_PATH})
endif(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
if(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))
	link_directories(${USER_LIBRARY_PATH})
endif(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))

set(BATTLED2_SRC src/battled2.cpp src/widgets.cpp)
add_executable(battled2 ${BATTLED2_SRC})

include_directories("include")

if(APPLE)
	FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
	FIND_LIBRARY(FOUNDATION_LIBRARY Foundation)
	FIND_LIBRARY(APPSERVICES_LIBRARY ApplicationServices)
	FIND_LIBRARY(APPKIT_LIBRARY AppKit)
	FIND_LIBRARY(IOKIT_LIBRARY IOKit)
	FIND_LIBRARY(AUDIOTOOLBOX_LIBRARY AudioToolbox)
	MARK_AS_ADVANCED(OPENGL_LIBRARY)
	MARK_AS_ADVANCED(FOUNDATION_LIBRARY)
	MARK_AS_ADVANCED(APPSERVICES_LIBRARY)
	MARK_AS_ADVANCED(IOKIT_LIBRARY)
	SET(EXTRA_LIBS ${IOKIT_LIBRARY} ${OPENGL_LIBRARY} ${FOUNDATION_LIBRARY} ${APPSERVICES_LIBRARY} ${APPKIT_LIBRARY} ${AUDIOTOOLBOX_LIBRARY} "png")
else(APPLE) # Windows
	SET(EXTRA_LIBS "opengl32" "dxguid" "dinput8" "dsound" "d3d9" "kernel32" "user32" "gdi32" "comdlg32" "ole32" "winmm" "psapi" "gdiplus" "uuid" "shlwapi")
endif(APPLE)

# FIXME: add Linux/X
	
target_link_libraries(
	battled2 
	tgui2
	atlas
	wrap
	allegro_image${ALLEG_SUFFIX}
	allegro_ttf${ALLEG_SUFFIX}
	allegro_font${ALLEG_SUFFIX}
	allegro_primitives${ALLEG_SUFFIX}
	allegro_color${ALLEG_SUFFIX}
	allegro_dialog${ALLEG_SUFFIX}
	allegro${ALLEG_SUFFIX}
	allegro_main${ALLEG_SUFFIX}
	freetype
	z
	${EXTRA_LIBS}
)


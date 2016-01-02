cmake_minimum_required(VERSION 2.6)
project(skeled CXX)

set(ALLEG_SUFFIX "-static")

if(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
	include_directories(${USER_INCLUDE_PATH})
endif(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
if(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))
	link_directories(${USER_LIBRARY_PATH})
endif(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))

include_directories("include")

set(SKELED_SRC skeled.cpp)
add_executable(skeled ${SKELED_SRC})

if(WIN32)
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT")
endif()

target_link_libraries(
	skeled
	crystalpicnic-engine
	wrap
	my_load_bitmap
	atlas
	tgui2
	allegro_monolith-static
	lua5.2
	freetype_static
	zlibstatic
	bassmidi
	bass
	poly2tri
	"opengl32" "dxguid" "dinput8" "xinput" "dsound" "d3d9" "kernel32" "user32" "gdi32" "comdlg32" "ole32" "winmm" "psapi" "gdiplus" "uuid" "shlwapi"
)

cmake_minimum_required(VERSION 2.6)
project(tgui2 CXX)

# Set USER_INCLUDE_PATH as may be necessary
# e.g., -DUSER_INCLUDE_PATH=/home/trent/local/include

if(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
	include_directories(${USER_INCLUDE_PATH})
endif(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))

if(WIN32)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

set(SOURCES tgui2.cpp tgui2_widgets.cpp)

add_library(tgui2 STATIC ${SOURCES})

target_link_libraries(tgui2 "allegro_monolith")
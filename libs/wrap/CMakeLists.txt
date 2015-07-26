cmake_minimum_required(VERSION 2.6)
project(wrap CXX)

set(ALLEG_SUFFIX "-static")

if(APPLE)
	SET(CMAKE_CXX_COMPILER "g++")
	set(CMAKE_CXX_FLAGS "-stdlib=libstdc++")
endif()

if(DEBUG)
	SET(LIB_NAME "wrap-debug")
	if(WIN32)
		SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	endif()
else()
	SET(LIB_NAME "wrap")
	if(WIN32)
		SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	endif()
endif()

if(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
	include_directories(${USER_INCLUDE_PATH})
endif(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))

include_directories("include")

set(WRAP_SRC src/wrap.cpp)
add_library(${LIB_NAME} STATIC ${WRAP_SRC})
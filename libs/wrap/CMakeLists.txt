cmake_minimum_required(VERSION 2.6)
project(wrap CXX)

set(ALLEG_SUFFIX "-static")

if(APPLE)
	SET(CMAKE_CXX_COMPILER "g++")
	set(CMAKE_CXX_FLAGS "-stdlib=libstdc++")
endif()

if(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
	include_directories(${USER_INCLUDE_PATH})
endif(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))

include_directories("include")

set(WRAP_SRC src/wrap.cpp)
add_library(wrap STATIC ${WRAP_SRC})


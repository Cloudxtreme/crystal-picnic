cmake_minimum_required(VERSION 2.6)
project(CrystalPicnicLauncher)

set(ALLEG_SUFFIX "-static")

if(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
	include_directories(${USER_INCLUDE_PATH})
endif(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
if(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))
	link_directories(${USER_LIBRARY_PATH})
endif(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))

ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_SOURCE_DIR}/LAUNCHER.cxx" COMMAND "C:\\Users\\trent\\bin\\fluid" ARGS "-c" "${CMAKE_SOURCE_DIR}/LAUNCHER.fl" DEPENDS "${CMAKE_SOURCE_DIR}/LAUNCHER.fl" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" PRE_BUILD)
ADD_CUSTOM_COMMAND(OUTPUT "CrystalPicnicLauncher.exe" COMMAND "upx" ARGS "${CMAKE_BINARY_DIR}/Release/CrystalPicnicLauncher.exe" POST_BUILD)

set(LAUNCHER_SRC "${CMAKE_SOURCE_DIR}/LAUNCHER.cxx" "${CMAKE_SOURCE_DIR}/../src/config.cpp" "${CMAKE_SOURCE_DIR}/../src/snprintf.c")
add_executable(CrystalPicnicLauncher ${LAUNCHER_SRC})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_USE_MATH_DEFINES /DALLEGRO_STATICLINK /EHsc /D_WIN32_WINNT=0x501 /DBUILDING_LAUNCHER")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /SUBSYSTEM:WINDOWS")

target_link_libraries(
	CrystalPicnicLauncher
	allegro-static
	fltk
	uuid
	comctl32
	opengl32
	winmm
	kernel32
	psapi
	shlwapi
	ole32
	xinput
	${CMAKE_SOURCE_DIR}/icon.res.lib
)


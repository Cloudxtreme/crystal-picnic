project(battled2)

set(BATTLED_SRC battled2.cpp widgets.cpp)

add_executable(battled2 ${BATTLED_SRC})
set_target_properties(battled2 PROPERTIES COMPILE_FLAGS "-g")

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
	SET(EXTRA_LIBS ${IOKIT_LIBRARY} ${OPENGL_LIBRARY} ${FOUNDATION_LIBRARY} ${APPSERVICES_LIBRARY} /Users/trent/code/local/lib/libfreetype.a ${APPKIT_LIBRARY} ${AUDIOTOOLBOX_LIBRARY} z png jpeg)
	target_link_libraries(battled2 
			/Users/trent/code/local/lib/libphysfs.a
			/Users/trent/code/local/lib/liblua5.2.a
			/Users/trent/code/local/lib/libz.a
			/Users/trent/code/local/lib/libpng.a
	/Users/trent/code/crystalpicnic/build/libcrystalpicnic-engine.a ${EXTRA_LIBS} allegro_physfs${ALLEG_SUFFIX} allegro_image${ALLEG_SUFFIX} allegro_ttf${ALLEG_SUFFIX} allegro_font${ALLEG_SUFFIX} allegro_primitives${ALLEG_SUFFIX} allegro_color${ALLEG_SUFFIX} allegro_dialog${ALLEG_SUFFIX} allegro_memfile${ALLEG_SUFFIX} allegro${ALLEG_SUFFIX} allegro_main${ALLEG_SUFFIX} bass bass_fx tgui2)
else(APPLE)
	target_link_libraries(battled2 c:/Users/trent/code/crystalpicnic/build/libcrystalpicnic-engine.a tgui2 allegro_image${ALLEG_SUFFIX} allegro_ttf${ALLEG_SUFFIX} allegro_font${ALLEG_SUFFIX} allegro_primitives${ALLEG_SUFFIX} allegro_color${ALLEG_SUFFIX} allegro_dialog${ALLEG_SUFFIX} allegro_memfile${ALLEG_SUFFIX} allegro${ALLEG_SUFFIX} allegro_main${ALLEG_SUFFIX} "opengl32" "dxguid" "dinput8" "dsound" "d3d9" "kernel32" "user32" "gdi32" "comdlg32" "ole32" "winmm" "psapi" "gdiplus" "uuid" "shlwapi" "freetype" "bass" "bassmidi" "opengl32" lua5.2 z "poly2tri")
endif(APPLE)


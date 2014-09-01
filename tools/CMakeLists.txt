project(projects)

if(APPLE)
   set(EXTRA_ALLEGRO_LIBS allegro_main${ALLEG_SUFFIX})
else(APPLE)
   set(EXTRA_ALLEGRO_LIBS )
endif(APPLE)

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
	SET(ALLEGRO_LIBS ${IOKIT_LIBRARY} ${OPENGL_LIBRARY} ${FOUNDATION_LIBRARY} ${APPSERVICES_LIBRARY} /Users/trent/code/local/lib/libfreetype.a ${APPKIT_LIBRARY} ${AUDIOTOOLBOX_LIBRARY} allegro${ALLEG_SUFFIX} allegro_memfile${ALLEG_SUFFIX} allegro_primitives${ALLEG_SUFFIX} allegro_image${ALLEG_SUFFIX} ${EXTRA_ALLEGRO_LIBS} /Users/trent/code/local/lib/libz.a /Users/trent/code/local/lib/libpng.a)
else(APPLE)
	if(WIN32)
		set(EXTRA_LIBS "opengl32" "dxguid" "dinput8" "dsound" "d3d9" "kernel32" "user32" "gdi32" "comdlg32" "ole32" "winmm" "psapi" "gdiplus" "uuid" "shlwapi")
		if(GNU)
			set(ZLIB_LIB_NAME "z")
		else()
			set(ZLIB_LIB_NAME "zlibstatic")
		endif()
		set(ALLEGRO_LIBS allegro${ALLEG_SUFFIX} allegro_primitives${ALLEG_SUFFIX} allegro_dialog${ALLEG_SUFFIX} allegro_image${ALLEG_SUFFIX} ${EXTRA_ALLEGRO_LIBS} freetype ${ZLIB_LIB_NAME} ${EXTRA_LIBS})
	else(WIN32)
		if(RASPBERRYPI)
			set(ALLEGRO_LIBS
				allegro_ttf${ALLEG_SUFFIX}
				allegro_font${ALLEG_SUFFIX}
				allegro_primitives${ALLEG_SUFFIX}
				allegro_image${ALLEG_SUFFIX}
				allegro_color${ALLEG_SUFFIX}
				allegro_memfile${ALLEG_SUFFIX}
				allegro${ALLEG_SUFFIX}
				${ALLEGRO_MAIN_LIB}
				m pthread SM ICE X11 Xext Xcursor Xinerama Xrandr Xpm bcm_host GLESv2 EGL
				/usr/lib/arm-linux-gnueabihf/libpng.a
				/usr/lib/arm-linux-gnueabihf/libfreetype.a
				/usr/lib/arm-linux-gnueabihf/libz.a
			)
		else(RASPBERRYPI)
			set(ALLEGRO_LIBS
				allegro_ttf${ALLEG_SUFFIX}
				allegro_font${ALLEG_SUFFIX}
				allegro_primitives${ALLEG_SUFFIX}
				allegro_image${ALLEG_SUFFIX}
				allegro_color${ALLEG_SUFFIX}
				allegro_memfile${ALLEG_SUFFIX}
				allegro${ALLEG_SUFFIX}
				${ALLEGRO_MAIN_LIB}
				m pthread SM ICE X11 Xext Xcursor Xinerama Xrandr Xpm GL GLU
#				/usr/lib/x86_64-linux-gnu/libpng.a
#				/usr/lib/x86_64-linux-gnu/libfreetype.a
#				/usr/lib/x86_64-linux-gnu/libz.a
				/usr/lib/libpng.a
				/usr/lib/libfreetype.a
				/usr/lib/libz.a
			)
		endif()
	endif(WIN32)
endif(APPLE)

function(add_tool target libs)
   add_executable(${target} ${ARGN})
   target_link_libraries(${target} ${libs})
endfunction(add_tool)

#add_tool(packarea "" packarea.cpp)

#add_tool(chunk "${ALLEGRO_LIBS}" chunk_bmp3_16bit.cpp)
#add_tool(level_optimizer "${ALLEGRO_LIBS}" level_optimizer.cpp)
add_tool(packtiles2 "${ALLEGRO_LIBS}" packtiles2.cpp)
add_tool(2cpi "${ALLEGRO_LIBS}" 2cpi.cpp)
#add_tool(mkheightmap "${ALLEGRO_LIBS}" mkheightmap.cpp)

if(NOT RASPBERRYPI)
	if(NOT WIN32 AND NOT APPLE)
		#add_tool(lineed "${ALLEGRO_LIBS};allegro_dialog${ALLEG_SUFFIX};gtkglext-x11-1.0;gtk-x11-2.0;gdk-x11-2.0;atk-1.0;gio-2.0;pangoft2-1.0;pangocairo-1.0;gdk_pixbuf-2.0;cairo;pango-1.0;freetype;fontconfig;gobject-2.0;glib-2.0" lineed.cpp)
	else(NOT WIN32 AND NOT APPLE)
		add_tool(lineed "${ALLEGRO_LIBS};allegro_dialog${ALLEG_SUFFIX}" lineed.cpp)
	endif(NOT WIN32 AND NOT APPLE)
endif()

if(APPLE)
add_tool(boneed "${ALLEGRO_LIBS};allegro_ttf${ALLEG_SUFFIX};allegro_font${ALLEG_SUFFIX};z" boneed.cpp)
else(APPLE)
add_tool(boneed "allegro_ttf${ALLEG_SUFFIX};allegro_font${ALLEG_SUFFIX};${ALLEGRO_LIBS}" boneed.cpp)
endif(APPLE)

#add_tool(weaponed "${MAIN_LIBS}" weaponed.cpp)

#add_tool(splitbitmap "${ALLEGRO_LIBS}" splitbitmap.cpp)

#add_tool(mk8bitshadow "${ALLEGRO_LIBS};z" mk8bitshadow.cpp)

#add_tool(mkmask "${ALLEGRO_LIBS}" mkmask.cpp)

#add_tool(shadow_edit "${ALLEGRO_LIBS};z" shadow_edit.cpp)

add_tool(rmalpha "${ALLEGRO_LIBS}" rmalpha.cpp)

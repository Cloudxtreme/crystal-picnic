#include <allegro5/allegro.h>

#ifdef ALLEGRO_WINDOWS

#include <cstdio>
#include "direct3d.h"

namespace Direct3D
{

bool lost = false;
bool got_display_lost = false;
bool got_display_found = false;

void lost_callback(ALLEGRO_DISPLAY *display)
{
	(void)display;

	/*
	if (!got_display_found) {
		return;
	}
	*/
	
	got_display_lost = true;
	lost = true;
}

void found_callback(ALLEGRO_DISPLAY *display)
{
	(void)display;

	got_display_found = true;
	lost = false;
}

} // end namespace Direct3D

#endif


#ifndef DIRECT3D_H
#define DIRECT3D_H

#include <allegro5/allegro.h>

#ifdef ALLEGRO_WINDOWS

#include <allegro5/allegro_direct3d.h>

namespace Direct3D
{

void lost_callback(ALLEGRO_DISPLAY *display);
void found_callback(ALLEGRO_DISPLAY *display);

extern bool lost;
extern bool got_display_lost;
extern bool got_display_found;

} // end namespace Direct3D

#endif // ALLEGRO_WINDOWS

#endif // DIRECT3D_H

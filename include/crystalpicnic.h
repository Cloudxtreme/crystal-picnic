#ifndef CRYSTALPICNIC_H
#define CRYSTALPICNIC_H

#ifdef _WIN32
#define _WIN32_IE 0x400
#endif

#include <iostream>
#include "main.h"
#include "general.h"
#include "error.h"

// This must be included for magic main to work
#include <allegro5/allegro.h>

#ifndef M_PI
#define M_PI ALLEGRO_PI
#endif

#ifdef ALLEGRO_WINDOWS
#define mkdir(a, b) mkdir(a)
#endif

#endif

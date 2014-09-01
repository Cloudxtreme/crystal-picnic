#ifndef MUSIC_H
#define MUSIC_H

#include <allegro5/allegro.h>

#include <vector>
#include <string>
#include <map>

#include "general.h"
#include "engine.h"

namespace Music {

void init();
void stop();
void play(std::string name, float volume = 1.0, bool loop = true);
std::string get_playing();
float get_volume();
void set_volume(float volume);
bool is_playing();
void ramp_up(double time);
void ramp_down(double time);
void shutdown();

}

#ifdef FIRETV
extern "C" {

	void crystalpicnic_pause_sound();
	void crystalpicnic_resume_sound();

}
#endif

#endif // MUSIC_H

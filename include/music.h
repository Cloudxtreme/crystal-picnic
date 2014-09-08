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

extern "C" {
	void switch_music_out(void);
	void switch_music_in(void);

}

#endif // MUSIC_H

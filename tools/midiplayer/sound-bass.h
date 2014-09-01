#ifndef SOUND_BASS_H
#define SOUND_BASS_H

#include <bass.h>

#include <string>

void init_sound();
void shutdown_sound();
HSTREAM load_midi(std::string filename, bool loop);
void set_midi_volume(HSTREAM s, float volume);
void play_midi(HSTREAM s, float volume);
void stop_midi(HSTREAM s);
void set_midi_tempo(HSTREAM s, float tempo);
HFX add_reverb(HSTREAM s);
void remove_fx(HSTREAM s, HFX fx);

#endif

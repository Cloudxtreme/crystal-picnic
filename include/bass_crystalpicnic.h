#ifndef BASS_CRYSTALPICNIC_H
#define BASS_CRYSTALPICNIC_H

#include <bass.h>

namespace BASS
{

void init();

HSAMPLE load_sample(const char *name, bool loop);
void play_sample(HSAMPLE s, float volume, float pan, float speed);
void adjust_sample(HSAMPLE s, float vol, float pan, float speed);
void stop_sample(HSAMPLE s);
void destroy_sample(HSAMPLE s);

HSAMPLE load_sample_loop(const char *name, bool loop);
void play_sample_loop(HSAMPLE s, float volume, float pan, float speed);
void adjust_sample_loop(HSAMPLE s, float vol, float pan, float speed);
void stop_sample_loop(HSAMPLE s);
void destroy_sample_loop(HSAMPLE s);

void shutdown();

}

#endif

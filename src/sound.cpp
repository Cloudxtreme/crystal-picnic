#include "crystalpicnic.h"
#include "sound.h"
#include "bass_crystalpicnic.h"

ALLEGRO_DEBUG_CHANNEL("CrystalPicnic")

namespace Sound {

Sample *load(std::string filename, bool loop)
{
	Sample *sample = new Sample;

	if (loop) {
		sample->sample = BASS::load_sample_loop(filename.c_str(), loop);
		BASS_ChannelGetAttribute(
			sample->sample,
			BASS_ATTRIB_FREQ,
			&sample->freq
		);
	}
	else {
		sample->sample = BASS::load_sample(filename.c_str(), loop);
		BASS_ChannelGetAttribute(
			BASS_SampleGetChannel(sample->sample, false),
			BASS_ATTRIB_FREQ,
			&sample->freq
		);
	}

	sample->is_set = false;
	sample->loop = loop;

	return sample;
}

Sample *load_set(std::string filename, std::string extension)
{
	Sample *sample = new Sample;

	for (int i = 1; i < 256; i++) {
		char buf[1000];
		sprintf(buf, "%s%d.%s", filename.c_str(), i, extension.c_str());
		HSAMPLE s = BASS::load_sample(buf, false);
		if (!s)
			break;
		sample->set.push_back(s);
		if (sample->set.size() == 1) {
			BASS_ChannelGetAttribute(
				BASS_SampleGetChannel(sample->set[sample->set.size()-1], false),
				BASS_ATTRIB_FREQ,
				&sample->freq
			);
		}
	}

	sample->is_set = true;
	sample->loop = false;
	
	return sample;
}

void destroy(Sample *sample)
{
	if (!sample->is_set) {
		if (sample->loop) {
			BASS::destroy_sample_loop(sample->sample);
		}
		else {
			BASS::destroy_sample(sample->sample);
		}
	}
	else {
		for (unsigned int i = 0; i < sample->set.size(); i++)
			BASS::destroy_sample(sample->set[i]);
	}
	delete sample;
}

void play(Sample *sample, float volume, float pan, float speed)
{
	if (!sample->is_set) {
		if (sample->loop) {
			BASS::play_sample_loop(sample->sample, cfg.sfx_volume * volume, pan, speed * sample->freq);
		}
		else {
			BASS::play_sample(sample->sample, cfg.sfx_volume * volume, pan, speed * sample->freq);
		}
	}
	else {
		BASS::play_sample(sample->set[General::rand()%sample->set.size()], cfg.sfx_volume * volume, pan, speed * sample->freq);
	}
}

void play(Sample *sample)
{
	play(sample, cfg.sfx_volume, 0.0f, 1);
}

void adjust(Sample *sample, float volume, float pan, float speed)
{
	if (sample->loop) {
		BASS::adjust_sample_loop(sample->sample, cfg.sfx_volume * volume, pan, speed * sample->freq);
	}
	else {
		BASS::adjust_sample(sample->sample, cfg.sfx_volume * volume, pan, speed * sample->freq);
	}
}

void stop(Sample *s)
{
	if (s->loop) {
		BASS::stop_sample_loop(s->sample);
	}
	else {
		BASS::stop_sample(s->sample);
	}
}

} // end namespace


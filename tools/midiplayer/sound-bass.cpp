#include <allegro5/allegro.h>

#include <bass.h>
#include <bass_fx.h>
#include <bassmidi.h>
static HSOUNDFONT sf2;

#include "sound-bass.h"

void init_sound()
{
	BASS_Init(-1, 44100, 0, NULL, NULL);
#ifdef ALLEGRO_WINDOWS
	BASS_PluginLoad("bassflac.dll", 0);
#else // OS X
	BASS_PluginLoad("libbassflac.dylib", 0);
#endif
	sf2 = BASS_MIDI_FontInit("retro.sf2pack", 0);
}

void shutdown_sound()
{
	BASS_Free();
}

HSTREAM load_midi(std::string filename, bool loop)
{
	HSTREAM s = BASS_MIDI_StreamCreateFile(false, filename.c_str(), 0, 0, (loop ? BASS_SAMPLE_LOOP : 0) | BASS_STREAM_DECODE, 0);

	BASS_MIDI_FONT f;
	f.font = sf2;
	f.preset = -1;
	f.bank = 0;

	BASS_MIDI_StreamSetFonts(s, &f, 1);

	HSTREAM s2 = BASS_FX_TempoCreate(s, (loop ? BASS_SAMPLE_LOOP : 0));

	return s2;
}

void set_midi_volume(HSTREAM s, float volume)
{
	BASS_ChannelSetAttribute(s, BASS_ATTRIB_VOL, volume);
}

void play_midi(HSTREAM s, float volume)
{
	set_midi_volume(s, volume);
	BASS_ChannelPlay(s, FALSE);
}

void stop_midi(HSTREAM s)
{
	BASS_ChannelStop(s);
}

void set_midi_tempo(HSTREAM s, float tempo)
{
	BASS_ChannelSetAttribute(s, BASS_ATTRIB_TEMPO, tempo);
}

HFX add_reverb(HSTREAM s)
{
	HFX fx = BASS_ChannelSetFX(s, BASS_FX_DX8_REVERB, 1);
	return fx;
}

void remove_fx(HSTREAM s, HFX fx)
{
	BASS_ChannelRemoveFX(s, fx);
}


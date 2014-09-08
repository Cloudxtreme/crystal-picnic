#include "crystalpicnic.h"
#include "music.h"
#include "bass_crystalpicnic.h"
#include "bass_fileprocs.h"
#include "bassmidi.h"

#include "hqm.h"

#ifdef ALLEGRO_ANDROID
#include "android.h"
#include <jni.h>
#endif

static std::string current_music = "";
static float current_volume = 1.0f;
static bool audio_ready;
static HSTREAM music = 0;
static bool music_playing = false;
static HSOUNDFONT sf2;
static unsigned char *delete_bytes = NULL;


static BASS_FILEPROCS fileprocs;

namespace Music {

void init()
{
	fileprocs.close = BASS_FileProcs::close;
	fileprocs.length = BASS_FileProcs::len;
	fileprocs.read = BASS_FileProcs::read;
	fileprocs.seek = BASS_FileProcs::seek;

	BASS::init();

	BASS_SetConfig(BASS_CONFIG_MIDI_VOICES, 20);
	BASS_SetConfig(BASS_CONFIG_MIDI_COMPACT, FALSE);

	ALLEGRO_FILE *file = engine->get_cpa()->load("music/retro.sf2");
	if (file) {
		sf2 = BASS_MIDI_FontInitUser(
			&fileprocs,
			file,
			0
		);

		audio_ready = true;
	}
}

void stop()
{
	if (cfg.music_off) return;
	if (!music_playing) return;
	BASS_StreamFree(music);
	music_playing = false;
}

void play(std::string filename, float volume, bool loop)
{
	if (!audio_ready) return;

	if (music_playing && filename == current_music) {
		return;
	}

	current_music = filename;
	current_volume = cfg.music_volume * volume;

	if (music) {
		Music::ramp_down(0.5);
		al_rest(0.5);
		BASS_StreamFree(music);
		delete[] delete_bytes;
		delete_bytes = NULL;
	}
	
	if (cfg.music_off) return;

	bool is_ogg;

	if (hqm_get_status(NULL) == HQM_STATUS_COMPLETE) {
		std::string s = filename.substr(6);
		s = General::get_download_path() + "/" + s.replace(s.length()-3, 3, "ogg");
		if (al_filename_exists(s.c_str())) {
			is_ogg = true;
			filename = s;
		}
		else {
			is_ogg = false;
		}
	}
	else {
		is_ogg = false;
	}

	int sz;
	unsigned char *bytes;

	if (is_ogg) {
		bytes = General::slurp_real_file(filename, &sz, false, false);
	}
	else {
		bytes = General::slurp(filename, &sz, false, false);
	}

	if (bytes) {
		if (strstr(filename.c_str(), ".ogg")) {
			music = BASS_StreamCreateFile(
				TRUE,
				bytes,
				0,
				sz,
				(loop ? BASS_SAMPLE_LOOP : 0)
			);

			BASS_ChannelSetAttribute(music, BASS_ATTRIB_VOL, 0.0f);
			BASS_ChannelPlay(music, FALSE);
			ramp_up(0.5);

			delete_bytes = bytes;

			music_playing = true;
		}
		else {
			music = BASS_MIDI_StreamCreateFile(
				TRUE,
				bytes,
				0,
				sz,
				(loop ? BASS_SAMPLE_LOOP : 0),
				0
			);

			delete[] bytes;
			
			BASS_MIDI_FONT f;
			f.font = sf2;
			f.preset = -1;
			f.bank = 0;

			BASS_MIDI_StreamSetFonts(music, &f, 1);
			BASS_MIDI_StreamLoadSamples(music);

			
			HFX fx = 0;
			if (cfg.reverb) {
				fx = BASS_ChannelSetFX(music, BASS_FX_DX8_REVERB, 1);
			}

			char basename[1000];
			for (int i = strlen(filename.c_str())-1; i >= 0; i--) {
				if (filename.c_str()[i] == '/' || filename.c_str()[i] == '\\') {
					i++;
					for (int j = i; filename.c_str()[j] != '.'; j++) {
						basename[j-i] = filename.c_str()[j];
						basename[j-i+1] = 0;
					}
					break;
				}
			}

			if (cfg.reverb) {
				std::string basename_string = basename;
				float val;
				bool set = true;
				if (basename_string == "abw") {
					val = 0.1f;
				}
				else if (basename_string == "abw2") {
					val = 0.15f;
				}
				else if (basename_string == "boss") {
					val = 0.1f;
				}
				else if (basename_string == "castle") {
					val = 0.15f;
				}
				else if (basename_string == "game_over") {
					val = 0.4f;
				}
				else if (basename_string == "map") {
					val = 0.05f;
				}
				else if (basename_string == "old_forest") {
					val = 0.4f;
				}
				else if (basename_string == "other") {
					val = 0.2f;
				}
				else if (basename_string == "river_town") {
					val = 0.4f;
				}
				else if (basename_string == "stonecrater") {
					val = 0.1f;
				}
				else if (basename_string == "title") {
					val = 0.1f;
				}
				else if (basename_string == "whack_a_skunk") {
					val = 0.05f;
				}
				else {
					set = false;
				}
				if (set) {
					val *= 2999999;
					val = val + 1;
					val /= 1000;
					BASS_DX8_REVERB reverb_params;
					BASS_FXGetParameters(fx, (void *)&reverb_params);
					reverb_params.fReverbTime = val;
					BASS_FXSetParameters(fx, (void *)&reverb_params);
				}
			}

			BASS_ChannelSetAttribute(music, BASS_ATTRIB_VOL, 0.0f);
			BASS_ChannelPlay(music, FALSE);
			ramp_up(0.5);

			music_playing = true;
		}
	}
	else {
		music = 0;
		music_playing = false;
	}
}

std::string get_playing()
{
	return current_music;
}

float get_volume()
{
	return current_volume;
}

void set_volume(float volume)
{
	current_volume = volume;
	BASS_ChannelSetAttribute(music, BASS_ATTRIB_VOL, volume);
}

bool is_playing()
{
	return music_playing;
}

void ramp_up(double time)
{
	BASS_ChannelSlideAttribute(music, BASS_ATTRIB_VOL, current_volume, time*1000);
}

void ramp_down(double time)
{
	BASS_ChannelSlideAttribute(music, BASS_ATTRIB_VOL, 0.0f, time*1000);
}

void shutdown()
{
	delete[] delete_bytes;
	delete_bytes = NULL;
	BASS_MIDI_FontFree(sf2);
}

} // end namespace Music


JNIEXPORT void JNICALL Java_com_nooskewl_crystalpicnic_MyBroadcastReceiver_pauseSound
  (JNIEnv *env, jobject obj)
{
	engine->switch_music_out();
}

JNIEXPORT void JNICALL Java_com_nooskewl_crystalpicnic_MyBroadcastReceiver_resumeSound
  (JNIEnv *env, jobject obj)
{
	engine->switch_music_in();
}


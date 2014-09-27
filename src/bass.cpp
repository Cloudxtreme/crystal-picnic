#include <allegro5/allegro.h>

// FIXME: new MinGW needs this
#ifdef ALLEGRO_WINDOWS
#include <windows.h>
#endif

#include <bass.h>
#include "bass_crystalpicnic.h"
#include "bass_fileprocs.h"
#include "general.h"
#include "engine.h"

ALLEGRO_DEBUG_CHANNEL("CrystalPicnic")

static BASS_FILEPROCS fileprocs;


static void CALLBACK MusicSyncProc(
	HSYNC handle, DWORD channel, DWORD data, void *user)
{
	if (!BASS_ChannelSetPosition(channel, 0, BASS_POS_BYTE))
		BASS_ChannelSetPosition(channel, 0, BASS_POS_BYTE);
}


namespace BASS
{

void init(void)
{
	fileprocs.close = BASS_FileProcs::close;
	fileprocs.length = BASS_FileProcs::len;
	fileprocs.read = BASS_FileProcs::read;
	fileprocs.seek = BASS_FileProcs::seek;

#ifdef ALLEGRO_IPHONE
	BASS_SetConfig(BASS_CONFIG_IOS_MIXAUDIO, 5);
#endif

#ifdef ALLEGRO_RASPBERRYPI
	if (!BASS_Init(cfg.audio_device, 22050, 0, NULL, NULL)) {
#else
	if (!BASS_Init(cfg.audio_device, 44100, 0, NULL, NULL)) {
#endif
		int code = BASS_ErrorGetCode();
				ALLEGRO_DEBUG("BASS_Init failed (%d). Failing or falling back", code);
	}

#ifdef ALLEGRO_RASPBERRPI_XXX
	//BASS_PluginLoad("libbassmidi.so", 0);
#endif
}

static HSTREAM get_decode_stream(const char *name, unsigned char **buf)
{
	int sz;
	*buf = General::slurp(
		name,
		&sz,
		false,
		true
	);
	if (*buf == NULL) {
		ALLEGRO_DEBUG("buf == NULL");
		return 0;
	}
	HSTREAM stream = BASS_StreamCreateFile(
		true,
		*buf,
		0,
		sz,
		BASS_STREAM_DECODE
	);
	return stream;
}

HSAMPLE load_sample(const char *name, bool loop)
{
	unsigned char *buf0;
	HSTREAM stream = get_decode_stream(name, &buf0);
	if (stream == 0) {
		return 0;
	}
	#define SZ (256*1024)
	int total_read = 0;
	int bufsize = 0;
	DWORD read = 0;
	unsigned char *buf = NULL;
	do {
		if (bufsize == 0) {
			bufsize += SZ;
			buf = (unsigned char *)malloc(SZ);
		}
		else {
			bufsize += SZ;
			buf = (unsigned char *)realloc(buf, bufsize);
		}
		read = BASS_ChannelGetData(stream, buf+total_read, SZ);
		total_read += read;
	} while (read == SZ);
	#undef SZ
	BASS_CHANNELINFO info;
	BASS_ChannelGetInfo(stream, &info);
	BASS_StreamFree(stream);
	HSAMPLE samp = BASS_SampleCreate(
		total_read,
		info.freq,
		info.chans,
		4,
		BASS_SAMPLE_OVER_POS | (loop ? BASS_SAMPLE_LOOP : 0)
	);
	BASS_SampleSetData(samp, buf);
	free(buf0);
	free(buf);

	return samp;
}

void play_sample(HSAMPLE s, float vol, float pan, float speed)
{
	HCHANNEL chan = BASS_SampleGetChannel(s, false);
	
	BASS_ChannelSetPosition(chan, BASS_POS_BYTE, 0);
	BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, vol);
	BASS_ChannelSetAttribute(chan, BASS_ATTRIB_PAN, pan);
	BASS_ChannelSetAttribute(chan, BASS_ATTRIB_FREQ, speed);

	BASS_ChannelPlay(chan, false);
}

void adjust_sample(HSAMPLE s, float vol, float pan, float speed)
{
	HCHANNEL chan = s;

	BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, vol);
	BASS_ChannelSetAttribute(chan, BASS_ATTRIB_PAN, pan);
	BASS_ChannelSetAttribute(chan, BASS_ATTRIB_FREQ, speed);
}

void stop_sample(HSAMPLE s)
{
	BASS_ChannelStop(s);
}

void destroy_sample(HSAMPLE s)
{
	BASS_SampleFree(s);
}

HSAMPLE load_sample_loop(const char *filename, bool loop)
{
	HSAMPLE sample;
	ALLEGRO_FILE *file = engine->get_cpa()->load(filename);
	sample = BASS_StreamCreateFileUser(
		STREAMFILE_NOBUFFER,
		0,
		&fileprocs,
		file
	);
	return sample;
}

void play_sample_loop(HSAMPLE s, float vol, float pan, float speed)
{
	adjust_sample_loop(s, vol, pan, speed);

	BASS_ChannelSetSync(s, BASS_SYNC_END | BASS_SYNC_MIXTIME,
		0, MusicSyncProc, 0);

	BASS_ChannelPlay(s, false);
}

void adjust_sample_loop(HSAMPLE s, float vol, float pan, float speed)
{
	BASS_ChannelSetAttribute(s, BASS_ATTRIB_VOL, vol);
	BASS_ChannelSetAttribute(s, BASS_ATTRIB_PAN, pan);
	BASS_ChannelSetAttribute(s, BASS_ATTRIB_FREQ, speed);
}

void stop_sample_loop(HSAMPLE s)
{
	BASS_ChannelStop(s);
}

void destroy_sample_loop(HSAMPLE s)
{
	BASS_StreamFree(s);
}

void shutdown(void)
{
	BASS_Free();
}

} // end namespace BASS

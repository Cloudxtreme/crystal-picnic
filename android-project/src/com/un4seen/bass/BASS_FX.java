package com.un4seen.bass;

public class BASS_FX
{
	// Error codes returned by BASS_ErrorGetCode()
	public static final int BASS_ERROR_FX_NODECODE = 4000;
	public static final int BASS_ERROR_FX_BPMINUSE = 4001;

	// Tempo / Reverse / BPM / Beat flag
	public static final int BASS_FX_FREESOURCE = 0x10000;

	// BASS_FX Version
	public static native int BASS_FX_GetVersion();

	// DSP channels flags
	public static final int BASS_BFX_CHANALL = -1;
	public static final int BASS_BFX_CHANNONE = 0;
	public static final int BASS_BFX_CHAN1 = 1;
	public static final int BASS_BFX_CHAN2 = 2;
	public static final int BASS_BFX_CHAN3 = 4;
	public static final int BASS_BFX_CHAN4 = 8;
	public static final int BASS_BFX_CHAN5 = 16;
	public static final int BASS_BFX_CHAN6 = 32;
	public static final int BASS_BFX_CHAN7 = 64;
	public static final int BASS_BFX_CHAN8 = 128;

	// if you have more than 8 channels, use this macro
	public static int BASS_BFX_CHANNEL_N(int n) { return (1<<((n)-1)); }

	// DSP effects
//	public static final int BASS_FX_BFX_ROTATE = 0x10000;
//	public static final int BASS_FX_BFX_ECHO = 0x10001;
//	public static final int BASS_FX_BFX_FLANGER = 0x10002;
	public static final int BASS_FX_BFX_VOLUME = 0x10003;
//	public static final int BASS_FX_BFX_PEAKEQ = 0x10004;
//	public static final int BASS_FX_BFX_REVERB = 0x10005;
//	public static final int BASS_FX_BFX_LPF = 0x10006;
//	public static final int BASS_FX_BFX_MIX = 0x10007;
//	public static final int BASS_FX_BFX_DAMP = 0x10008;
//	public static final int BASS_FX_BFX_AUTOWAH = 0x10009;
//	public static final int BASS_FX_BFX_ECHO2 = 0x1000a;
//	public static final int BASS_FX_BFX_PHASER = 0x1000b;
//	public static final int BASS_FX_BFX_ECHO3 = 0x1000c;
//	public static final int BASS_FX_BFX_CHORUS = 0x1000d;
//	public static final int BASS_FX_BFX_APF = 0x1000e;
//	public static final int BASS_FX_BFX_COMPRESSOR = 0x1000f;
//	public static final int BASS_FX_BFX_DISTORTION = 0x10010;
//	public static final int BASS_FX_BFX_COMPRESSOR2 = 0x10011;
//	public static final int BASS_FX_BFX_VOLUME_ENV = 0x10012;
//	public static final int BASS_FX_BFX_BQF = 0x10013;

	// Volume
	public static class BASS_BFX_VOLUME {
		public int lChannel;
		public float fVolume;
	}

	// tempo attributes (BASS_ChannelSet/GetAttribute)
	public static final int BASS_ATTRIB_TEMPO = 0x10000;
	public static final int BASS_ATTRIB_TEMPO_PITCH = 0x10001;
	public static final int BASS_ATTRIB_TEMPO_FREQ = 0x10002;

	// tempo attributes options
	public static final int BASS_ATTRIB_TEMPO_OPTION_USE_AA_FILTER = 0x10010;
	public static final int BASS_ATTRIB_TEMPO_OPTION_AA_FILTER_LENGTH = 0x10011;
	public static final int BASS_ATTRIB_TEMPO_OPTION_USE_QUICKALGO = 0x10012;
	public static final int BASS_ATTRIB_TEMPO_OPTION_SEQUENCE_MS = 0x10013;
	public static final int BASS_ATTRIB_TEMPO_OPTION_SEEKWINDOW_MS = 0x10014;
	public static final int BASS_ATTRIB_TEMPO_OPTION_OVERLAP_MS = 0x10015;
	public static final int BASS_ATTRIB_TEMPO_OPTION_PREVENT_CLICK = 0x10016;

	public static native int BASS_FX_TempoCreate(int chan, int flags);
	public static native int BASS_FX_TempoGetSource(int chan);
	public static native float BASS_FX_TempoGetRateRatio(int chan);

	// reverse attribute (BASS_ChannelSet/GetAttribute)
	public static final int BASS_ATTRIB_REVERSE_DIR = 0x11000;

	// playback directions
	public static final int BASS_FX_RVS_REVERSE = -1;
	public static final int BASS_FX_RVS_FORWARD = 1;

	public static native int BASS_FX_ReverseCreate(int chan, float dec_block, int flags);
	public static native int BASS_FX_ReverseGetSource(int chan);

	// bpm flags
	public static final int BASS_FX_BPM_MULT2 = 2;

	// translation options
	public static final int BASS_FX_BPM_TRAN_X2 = 0;
	public static final int BASS_FX_BPM_TRAN_2FREQ = 1;
	public static final int BASS_FX_BPM_TRAN_FREQ2 = 2;
	public static final int BASS_FX_BPM_TRAN_2PERCENT = 3;
	public static final int BASS_FX_BPM_TRAN_PERCENT2 = 4;

	public interface BPMPROCESSPROC
	{
		void BPMPROCESSPROC(int chan, float percent, Object user);
	}
	public interface BPMPROC
	{
		void BPMPROC(int chan, float bpm, Object user);
	}

	public static native float BASS_FX_BPM_DecodeGet(int chan, double startSec, double endSec, int minMaxBPM, int flags, BPMPROCESSPROC proc, Object user);
	public static native boolean BASS_FX_BPM_CallbackSet(int handle, BPMPROC proc, double period, int minMaxBPM, int flags, Object user);
	public static native boolean BASS_FX_BPM_CallbackReset(int handle);
	public static native float BASS_FX_BPM_Translate(int handle, float val2tran, int trans);
	public static native boolean BASS_FX_BPM_Free(int handle);

	public interface BPMBEATPROC
	{
		void BPMBEATPROC(int chan, double beatpos, Object user);
	}

	public static native boolean BASS_FX_BPM_BeatCallbackSet(int handle, BPMBEATPROC proc, Object user);
	public static native boolean BASS_FX_BPM_BeatCallbackReset(int handle);
	public static native boolean BASS_FX_BPM_BeatDecodeGet(int chan, double startSec, double endSec, int flags, BPMBEATPROC proc, Object user);
	public static native boolean BASS_FX_BPM_BeatSetParameters(int handle, float bandwidth, float centerfreq, float beat_rtime);
	public static native boolean BASS_FX_BPM_BeatGetParameters(int handle, Float bandwidth, Float centerfreq, Float beat_rtime);
	public static native boolean BASS_FX_BPM_BeatFree(int handle);
	
    static {
        System.loadLibrary("bass_fx");
    }
}

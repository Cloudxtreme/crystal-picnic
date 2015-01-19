#include <sstream>
#include <cstdio>

#include "snprintf.h"
#include "config.h"

ALLEGRO_DEBUG_CHANNEL("CrystalPicnic")

#ifdef BUILDING_LAUNCHER
const int MAX_PATH = 5000;

#define LOG noop

void noop(std::string s)
{
	(void)s;
}
#else
#include "general.h"
#define LOG General::log_message
#endif

#define CFG_FILENAME "config.ini"

Configuration cfg;

void Configuration::reset_keyboard_controls()
{
	cfg.key_left = ALLEGRO_KEY_LEFT;
	cfg.key_right = ALLEGRO_KEY_RIGHT;
	cfg.key_up = ALLEGRO_KEY_UP;
	cfg.key_down = ALLEGRO_KEY_DOWN;
	cfg.key_menu = ALLEGRO_KEY_ESCAPE;
	cfg.key_run = ALLEGRO_KEY_LSHIFT;
	cfg.key_switch = ALLEGRO_KEY_TAB;
	cfg.key_ability[0] = ALLEGRO_KEY_W;
	cfg.key_ability[1] = ALLEGRO_KEY_A;
	cfg.key_ability[2] = ALLEGRO_KEY_D;
	cfg.key_ability[3] = ALLEGRO_KEY_S;
}

void Configuration::reset_gamepad_controls()
{
#ifdef ALLEGRO_ANDROID
	cfg.joy_ability[0] = 3;
	cfg.joy_ability[1] = 2;
	cfg.joy_ability[2] = 1;
	cfg.joy_ability[3] = 0;
	cfg.joy_menu = 10;
	cfg.joy_switch = 5;
	cfg.joy_arrange_up = 4;
	cfg.joy_arrange_down = 5;
	cfg.joy_dpad_l = 6;
	cfg.joy_dpad_r = 7;
	cfg.joy_dpad_u = 8;
	cfg.joy_dpad_d = 9;
#else
	cfg.joy_ability[0] = 3;
	cfg.joy_ability[1] = 2;
	cfg.joy_ability[2] = 1;
	cfg.joy_ability[3] = 0;
#if defined __linux__
	cfg.joy_menu = 7;
	cfg.joy_switch = 5;
	cfg.joy_arrange_up = 4;
	cfg.joy_arrange_down = 5;
	cfg.joy_dpad_l = -1;
	cfg.joy_dpad_r = -1;
	cfg.joy_dpad_u = -1;
	cfg.joy_dpad_d = -1;
#elif defined ALLEGRO_MACOSX
	cfg.joy_menu = 8;
	cfg.joy_switch = 5;
	cfg.joy_arrange_up = 4;
	cfg.joy_arrange_down = 5;
	cfg.joy_dpad_l = -1;
	cfg.joy_dpad_r = -1;
	cfg.joy_dpad_u = -1;
	cfg.joy_dpad_d = -1;
#else
	cfg.joy_menu = 9;
	cfg.joy_switch = 4;
	cfg.joy_arrange_up = 5;
	cfg.joy_arrange_down = 4;
	cfg.joy_dpad_l = 11;
	cfg.joy_dpad_r = 10;
	cfg.joy_dpad_u = 13;
	cfg.joy_dpad_d = 12;
#endif
#endif
}

void Configuration::reset(void)
{
	difficulty = HARD;

	language = "English";

	vsync = true;
	screen_w = 1024;
	screen_h = 576;
	save_screen_w = screen_w;
	save_screen_h = screen_h;
	fullscreen = false;
#if defined ALLEGRO_ANDROID || defined ALLEGRO_IPHONE || defined ALLEGRO_RASPBERRYPI
	low_graphics = true;
#else
	low_graphics = false;
#endif
	water_shader = true;
	force_opengl = false;
	show_fps = false;
	linear_filtering = true;

	music_volume = 1.0f;
	sfx_volume = 1.0f;
	audio_device = -1;
	music_off = false;
#ifdef ALLEGRO_RASPBERRYPI
	reverb = false;
#else
	reverb = true;
#endif

	reset_keyboard_controls();
	reset_gamepad_controls();

	beat_game = false;

	debugmode = false;

	adapter = ALLEGRO_DEFAULT_DISPLAY_ADAPTER;

	loaded_w = screen_w;
	loaded_h = screen_h;
	loaded_fullscreen = fullscreen;
	loaded_force_opengl = force_opengl;
	loaded_linear_filtering = linear_filtering;
	loaded_reverb = reverb;
}

static std::string cfg_path(void)
{
	char tmp[5000];
	ALLEGRO_PATH *user_path = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
	snprintf(tmp, 5000, "%s/%s", al_path_cstr(user_path, ALLEGRO_NATIVE_PATH_SEP), CFG_FILENAME);
	al_destroy_path(user_path);
	return tmp;
}

bool Configuration::load(void)
{
	LOG("Reading configuration file.");

	ALLEGRO_CONFIG *acfg = al_load_config_file(cfg_path().c_str());
	if (!acfg) {
		LOG("Could not read configuration file.\n");
		return false;
	}

	const char *v_version = al_get_config_value(acfg, "miscellaneous", "version");
	int start_version = 1;
	if (v_version) {
		start_version = atoi(v_version);
		LOG("Read version: " + std::string(v_version));
	}
	else {
		LOG("Warning: version not present.");
	}

	const char *v_vsync = al_get_config_value(acfg, "gfx", "vsync");
	if (v_vsync) {
		vsync = !strcmp(v_vsync, "true");
		LOG("Read vsync: " + std::string(v_vsync));
	}
	else {
		LOG("Warning: vsync not present.");
	}

	const char *v_screen_w = al_get_config_value(acfg, "gfx", "screen_w");
	if (v_screen_w) {
		screen_w = atoi(v_screen_w);
		save_screen_w = screen_w;
		LOG("Read screen_w: " + std::string(v_screen_w));
	}
	else {
		LOG("Warning: screen_w not present.");
	}
	loaded_w = screen_w;

	const char *v_screen_h = al_get_config_value(acfg, "gfx", "screen_h");
	if (v_screen_h) {
		screen_h = atoi(v_screen_h);
		save_screen_h = screen_h;
		LOG("Read screen_h: " + std::string(v_screen_h));
	}
	else {
		LOG("Warning: screen_w not present.");
	}
	loaded_h = screen_h;

	const char *v_fullscreen = al_get_config_value(acfg, "gfx", "fullscreen");
	if (v_fullscreen) {
		fullscreen = !strcmp(v_fullscreen, "true");
		LOG("Read fullscreen: " + std::string(v_fullscreen));
	}
	else {
		LOG("Warning: fullscreen not present.");
	}
	loaded_fullscreen = fullscreen;

	const char *v_low_graphics = al_get_config_value(acfg, "gfx", "low_graphics");
	if (v_low_graphics) {
		low_graphics = !strcmp(v_low_graphics, "true");
		LOG("Read low_graphics: " + std::string(v_low_graphics));
	}
	else {
		LOG("Warning: low_graphics not present.");
	}

	const char *v_water_shader = al_get_config_value(acfg, "gfx", "water_shader");
	if (v_water_shader) {
		water_shader = !strcmp(v_water_shader, "true");
		LOG("Read water_shader: " + std::string(v_water_shader));
	}
	else {
		LOG("Warning: water_shader not present.");
	}

	const char *v_linear_filtering = al_get_config_value(acfg, "gfx", "linear_filtering");
	if (v_linear_filtering) {
		linear_filtering = !strcmp(v_linear_filtering, "true");
		LOG("Read linear_filtering: " + std::string(v_linear_filtering));
	}
	else {
		LOG("Warning: linear_filtering not present.");
	}
	loaded_linear_filtering = linear_filtering;

	const char *v_force_opengl = al_get_config_value(acfg, "gfx", "force_opengl");
	if (v_force_opengl) {
		force_opengl = !strcmp(v_force_opengl, "true");
		LOG("Read force_opengl: " + std::string(v_force_opengl));
	}
	else {
		LOG("Warning: force_opengl not present.");
	}
	loaded_force_opengl = force_opengl;

	const char *v_show_fps = al_get_config_value(acfg, "gfx", "show_fps");
	if (v_show_fps) {
		show_fps = !strcmp(v_show_fps, "true");
		LOG("Read show_fps: " + std::string(v_show_fps));
	}
	else {
		LOG("Warning: show_fps not present.");
	}

	const char *v_audio_device = al_get_config_value(acfg, "audio", "audio_device");
	if (v_audio_device) {
		audio_device = atoi(v_audio_device);
		LOG("Read audio_device: " + std::string(v_audio_device));
	}
	else {
		LOG("Warning: audio_device not present.");
	}

	const char *v_music_volume = al_get_config_value(acfg, "audio", "music_volume");
	if (v_music_volume) {
		music_volume = atof(v_music_volume);
		LOG("Read music_volume: " + std::string(v_music_volume));
	}
	else {
		LOG("Warning: music_volume not present.");
	}

	const char *v_sfx_volume = al_get_config_value(acfg, "audio", "sfx_volume");
	if (v_sfx_volume) {
		sfx_volume = atof(v_sfx_volume);
		LOG("Read sfx_volume: " + std::string(v_sfx_volume));
	}
	else {
		LOG("Warning: sfx_volume not present.");
	}

	const char *v_music_off = al_get_config_value(acfg, "audio", "music_off");
	if (v_music_off) {
		music_off = !strcmp(v_music_off, "true");
		LOG("Read music_off: " + std::string(v_music_off));
	}
	else {
		LOG("Warning: music_off not present.");
	}

	const char *v_reverb = al_get_config_value(acfg, "audio", "reverb");
	if (v_reverb) {
		reverb = !strcmp(v_reverb, "true");
		LOG("Read reverb: " + std::string(v_reverb));
	}
	else {
		LOG("Warning: reverb not present.");
	}
	loaded_reverb = reverb;

	const char *v_key_left = al_get_config_value(acfg, "input", "key_left");
	if (v_key_left) {
		key_left = atoi(v_key_left);
		LOG("Read key_left: " + std::string(v_key_left));
	}
	else {
		LOG("Warning: key_left not present.");
	}
	const char *v_key_right = al_get_config_value(acfg, "input", "key_right");
	if (v_key_right) {
		key_right = atoi(v_key_right);
		LOG("Read key_right: " + std::string(v_key_right));
	}
	else {
		LOG("Warning: key_right not present.");
	}
	const char *v_key_up = al_get_config_value(acfg, "input", "key_up");
	if (v_key_up) {
		key_up = atoi(v_key_up);
		LOG("Read key_up: " + std::string(v_key_up));
	}
	else {
		LOG("Warning: key_up not present.");
	}
	const char *v_key_down = al_get_config_value(acfg, "input", "key_down");
	if (v_key_down) {
		key_down = atoi(v_key_down);
		LOG("Read key_down: " + std::string(v_key_down));
	}
	else {
		LOG("Warning: key_down not present.");
	}
	const char *v_key_menu = al_get_config_value(acfg, "input", "key_menu");
	if (v_key_menu) {
		key_menu = atoi(v_key_menu);
		LOG("Read key_menu: " + std::string(v_key_menu));
	}
	else {
		LOG("Warning: key_menu not present.");
	}
	const char *v_key_run = al_get_config_value(acfg, "input", "key_run");
	if (v_key_run) {
		key_run = atoi(v_key_run);
		LOG("Read key_run: " + std::string(v_key_run));
	}
	else {
		LOG("Warning: key_run not present.");
	}
	const char *v_key_switch = al_get_config_value(acfg, "input", "key_switch");
	if (v_key_switch) {
		key_switch = atoi(v_key_switch);
		LOG("Read key_switch: " + std::string(v_key_switch));
	}
	else {
		LOG("Warning: key_switch not present.");
	}
	const char *v_key_ability0 = al_get_config_value(acfg, "input", "key_ability0");
	if (v_key_ability0) {
		key_ability[0] = atoi(v_key_ability0);
		LOG("Read key_ability[0]: " + std::string(v_key_ability0));
	}
	else {
		LOG("Warning: key_ability[0] not present.");
	}
	const char *v_key_ability1 = al_get_config_value(acfg, "input", "key_ability1");
	if (v_key_ability1) {
		key_ability[1] = atoi(v_key_ability1);
		LOG("Read key_ability[1]: " + std::string(v_key_ability1));
	}
	else {
		LOG("Warning: key_ability[1] not present.");
	}
	const char *v_key_ability2 = al_get_config_value(acfg, "input", "key_ability2");
	if (v_key_ability2) {
		key_ability[2] = atoi(v_key_ability2);
		LOG("Read key_ability[2]: " + std::string(v_key_ability2));
	}
	else {
		LOG("Warning: key_ability[2] not present.");
	}
	const char *v_key_ability3 = al_get_config_value(acfg, "input", "key_ability3");
	if (v_key_ability3) {
		key_ability[3] = atoi(v_key_ability3);
		LOG("Read key_ability[3]: " + std::string(v_key_ability3));
	}
	else {
		LOG("Warning: key_ability[3] not present.");
	}

#ifdef ALLEGRO_ANDROID
	/* Controls changed on Android when adding proper controller support to Allegro */
	if (start_version > 2) {
#else
	if (start_version != 1) {
#endif
		const char *v_joy_menu = al_get_config_value(acfg, "input", "joy_menu");
		if (v_joy_menu) {
			joy_menu = atoi(v_joy_menu);
			LOG("Read joy_menu: " + std::string(v_joy_menu));
		}
		else {
			LOG("Warning: joy_menu not present.");
		}
		const char *v_joy_switch = al_get_config_value(acfg, "input", "joy_switch");
		if (v_joy_switch) {
			joy_switch = atoi(v_joy_switch);
			LOG("Read joy_switch: " + std::string(v_joy_switch));
		}
		else {
			LOG("Warning: joy_switch not present.");
		}
		const char *v_joy_ability0 = al_get_config_value(acfg, "input", "joy_ability0");
		if (v_joy_ability0) {
			joy_ability[0] = atoi(v_joy_ability0);
			LOG("Read joy_ability[0]: " + std::string(v_joy_ability0));
		}
		else {
			LOG("Warning: joy_ability[0] not present.");
		}
		const char *v_joy_ability1 = al_get_config_value(acfg, "input", "joy_ability1");
		if (v_joy_ability1) {
			joy_ability[1] = atoi(v_joy_ability1);
			LOG("Read joy_ability[1]: " + std::string(v_joy_ability1));
		}
		else {
			LOG("Warning: joy_ability[1] not present.");
		}
		const char *v_joy_ability2 = al_get_config_value(acfg, "input", "joy_ability2");
		if (v_joy_ability2) {
			joy_ability[2] = atoi(v_joy_ability2);
			LOG("Read joy_ability[2]: " + std::string(v_joy_ability2));
		}
		else {
			LOG("Warning: joy_ability[2] not present.");
		}
		const char *v_joy_ability3 = al_get_config_value(acfg, "input", "joy_ability3");
		if (v_joy_ability3) {
			joy_ability[3] = atoi(v_joy_ability3);
			LOG("Read joy_ability[3]: " + std::string(v_joy_ability3));
		}
		else {
			LOG("Warning: joy_ability[3] not present.");
		}
		const char *v_joy_arrange_up = al_get_config_value(acfg, "input", "joy_arrange_up");
		if (v_joy_arrange_up) {
			joy_arrange_up = atoi(v_joy_arrange_up);
			LOG("Read joy_arrange_up: " + std::string(v_joy_arrange_up));
		}
		else {
			LOG("Warning: joy_arrange_up not present.");
		}
		const char *v_joy_arrange_down = al_get_config_value(acfg, "input", "joy_arrange_down");
		if (v_joy_arrange_down) {
			joy_arrange_down = atoi(v_joy_arrange_down);
			LOG("Read joy_arrange_down: " + std::string(v_joy_arrange_down));
		}
		else {
			LOG("Warning: joy_arrange_down not present.");
		}
		const char *v_joy_dpad_l = al_get_config_value(acfg, "input", "joy_dpad_l");
		if (v_joy_dpad_l) {
			joy_dpad_l = atoi(v_joy_dpad_l);
			LOG("Read joy_dpad_l: " + std::string(v_joy_dpad_l));
		}
		else {
			LOG("Warning: joy_dpad_l not present.");
		}
		const char *v_joy_dpad_r = al_get_config_value(acfg, "input", "joy_dpad_r");
		if (v_joy_dpad_r) {
			joy_dpad_r = atoi(v_joy_dpad_r);
			LOG("Read joy_dpad_r: " + std::string(v_joy_dpad_r));
		}
		else {
			LOG("Warning: joy_dpad_r not present.");
		}
		const char *v_joy_dpad_u = al_get_config_value(acfg, "input", "joy_dpad_u");
		if (v_joy_dpad_u) {
			joy_dpad_u = atoi(v_joy_dpad_u);
			LOG("Read joy_dpad_u: " + std::string(v_joy_dpad_u));
		}
		else {
			LOG("Warning: joy_dpad_u not present.");
		}
		const char *v_joy_dpad_d = al_get_config_value(acfg, "input", "joy_dpad_d");
		if (v_joy_dpad_d) {
			joy_dpad_d = atoi(v_joy_dpad_d);
			LOG("Read joy_dpad_d: " + std::string(v_joy_dpad_d));
		}
		else {
			LOG("Warning: joy_dpad_d not present.");
		}
	}

	const char *v_language = al_get_config_value(acfg, "miscellaneous", "language");
	if (v_language) {
		language = v_language;
		LOG("Read language: " + language);
	}
	else {
		LOG("Warning: language not present.");
	}

	const char *v_debugmode = al_get_config_value(acfg, "miscellaneous", "debugmode");
	if (v_debugmode) {
		debugmode = !strcmp(v_debugmode, "true");
		LOG("Read debugmode: " + std::string(v_debugmode));
	}
	else {
		LOG("Warning: debugmode not present.");
	}

	const char *v_important = al_get_config_value(acfg, "miscellaneous", "important");
	if (v_important) {
		beat_game = !strcmp(v_important, "true");
		LOG("Read important: " + std::string(v_important));
	}
	else {
		LOG("Warning: important not present.");
	}

	version = start_version;

	al_destroy_config(acfg);

	LOG("Done.");

	return true;
}

bool Configuration::save(void)
{
	LOG("Saving configuration file.");

	version = 3;

	save_screen_w = loaded_w;
	save_screen_h = loaded_h;
	fullscreen = loaded_fullscreen;
	force_opengl = loaded_force_opengl;
	linear_filtering = loaded_linear_filtering;
	reverb = loaded_reverb;

	ALLEGRO_CONFIG *acfg = al_create_config();
	if (!acfg) {
		LOG("Unable to save configuration file.");
		return false;
	}

	al_add_config_section(acfg, "gfx");
	std::stringstream ss;
	ss.str("");
	al_set_config_value(acfg, "gfx", "vsync", vsync ? "true" : "false");
	ss.str("");
	ss << save_screen_w;
	al_set_config_value(acfg, "gfx", "screen_w", ss.str().c_str());
	ss.str("");
	ss << save_screen_h;
	al_set_config_value(acfg, "gfx", "screen_h", ss.str().c_str());
	al_set_config_value(acfg, "gfx", "fullscreen", fullscreen ? "true" : "false");
	al_set_config_value(acfg, "gfx", "low_graphics", low_graphics ? "true" : "false");
	al_set_config_value(acfg, "gfx", "water_shader", water_shader ? "true" : "false");
	al_set_config_value(acfg, "gfx", "linear_filtering", linear_filtering ? "true" : "false");
	al_set_config_value(acfg, "gfx", "force_opengl", force_opengl ? "true" : "false");
	al_set_config_value(acfg, "gfx", "show_fps", show_fps ? "true" : "false");
	
	al_add_config_section(acfg, "audio");
	ss.str("");
	ss << audio_device;
	al_set_config_value(acfg, "audio", "audio_device", ss.str().c_str());
	ss.str("");
	ss << music_volume;
	al_set_config_value(acfg, "audio", "music_volume", ss.str().c_str());
	ss.str("");
	ss << sfx_volume;
	al_set_config_value(acfg, "audio", "sfx_volume", ss.str().c_str());
	al_set_config_value(acfg, "audio", "music_off", music_off ? "true" : "false");
	al_set_config_value(acfg, "audio", "reverb", reverb ? "true" : "false");
	
	al_add_config_section(acfg, "input");
	ss.str("");
	ss << key_left;
	al_set_config_value(acfg, "input", "key_left", ss.str().c_str());
	ss.str("");
	ss << key_right;
	al_set_config_value(acfg, "input", "key_right", ss.str().c_str());
	ss.str("");
	ss << key_up;
	al_set_config_value(acfg, "input", "key_up", ss.str().c_str());
	ss.str("");
	ss << key_down;
	al_set_config_value(acfg, "input", "key_down", ss.str().c_str());
	ss.str("");
	ss << key_ability[0];
	al_set_config_value(acfg, "input", "key_ability0", ss.str().c_str());
	ss.str("");
	ss << key_ability[1];
	al_set_config_value(acfg, "input", "key_ability1", ss.str().c_str());
	ss.str("");
	ss << key_ability[2];
	al_set_config_value(acfg, "input", "key_ability2", ss.str().c_str());
	ss.str("");
	ss << key_ability[3];
	al_set_config_value(acfg, "input", "key_ability3", ss.str().c_str());
	ss.str("");
	ss << key_menu;
	al_set_config_value(acfg, "input", "key_menu", ss.str().c_str());
	ss.str("");
	ss << key_run;
	al_set_config_value(acfg, "input", "key_run", ss.str().c_str());
	ss.str("");
	ss << key_switch;
	al_set_config_value(acfg, "input", "key_switch", ss.str().c_str());

	ss.str("");
	ss << joy_menu;
	al_set_config_value(acfg, "input", "joy_menu", ss.str().c_str());
	ss.str("");
	ss << joy_switch;
	al_set_config_value(acfg, "input", "joy_switch", ss.str().c_str());
	ss.str("");
	ss << joy_ability[0];
	al_set_config_value(acfg, "input", "joy_ability0", ss.str().c_str());
	ss.str("");
	ss << joy_ability[1];
	al_set_config_value(acfg, "input", "joy_ability1", ss.str().c_str());
	ss.str("");
	ss << joy_ability[2];
	al_set_config_value(acfg, "input", "joy_ability2", ss.str().c_str());
	ss.str("");
	ss << joy_ability[3];
	al_set_config_value(acfg, "input", "joy_ability3", ss.str().c_str());
	ss.str("");
	ss << joy_arrange_up;
	al_set_config_value(acfg, "input", "joy_arrange_up", ss.str().c_str());
	ss.str("");
	ss << joy_arrange_down;
	al_set_config_value(acfg, "input", "joy_arrange_down", ss.str().c_str());
	ss.str("");
	ss << joy_dpad_l;
	al_set_config_value(acfg, "input", "joy_dpad_l", ss.str().c_str());
	ss.str("");
	ss << joy_dpad_r;
	al_set_config_value(acfg, "input", "joy_dpad_r", ss.str().c_str());
	ss.str("");
	ss << joy_dpad_u;
	al_set_config_value(acfg, "input", "joy_dpad_u", ss.str().c_str());
	ss.str("");
	ss << joy_dpad_d;
	al_set_config_value(acfg, "input", "joy_dpad_d", ss.str().c_str());

	al_add_config_section(acfg, "miscellaneous");
	ss.str("");
	ss << version;
	al_set_config_value(acfg, "miscellaneous", "version", ss.str().c_str());
	al_set_config_value(acfg, "miscellaneous", "language", language.c_str());
	al_set_config_value(acfg, "miscellaneous", "debugmode", debugmode ? "true" : "false");
	al_set_config_value(acfg, "miscellaneous", "important", beat_game ? "true" : "false");

	bool ret = al_save_config_file(cfg_path().c_str(), acfg);

	al_destroy_config(acfg);

	return ret;
}

Configuration::Configuration(void)
{
}


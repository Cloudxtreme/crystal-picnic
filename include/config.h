#ifndef CONFIG_H
#define CONFIG_H

#include <allegro5/allegro5.h>
#include <string>

class Configuration {
public:
	enum Difficulty {
		EASY = 0,
		NORMAL = 1,
		HARD = 2
	};

	Difficulty difficulty;
	bool cancelled;

	bool vsync;
	int screen_w;
	int screen_h;
	int save_screen_w;
	int save_screen_h;
	float screens_w;
	float screens_h;
	bool fullscreen;
	bool opengl;
	bool force_opengl;
	bool low_graphics;
	bool water_shader;
	bool linear_filtering;
	bool show_fps;

	int loaded_w;
	int loaded_h;
	bool loaded_fullscreen;
	bool loaded_force_opengl;
	bool loaded_linear_filtering;

	int audio_device;
	float music_volume;
	float sfx_volume;
	bool music_off;
	bool reverb;

	bool loaded_reverb;

	std::string language;

	int joy_menu;
	int joy_switch;
	int joy_ability[4];
	int joy_arrange_up;
	int joy_arrange_down;
	int joy_dpad_l;
	int joy_dpad_r;
	int joy_dpad_u;
	int joy_dpad_d;

	int key_left;
	int key_right;
	int key_up;
	int key_down;
	int key_menu;
	int key_run;
	int key_switch;
	int key_ability[4];

	bool beat_game;

	int adapter;

	int version;

	void reset_keyboard_controls();
	void reset_gamepad_controls();
	void reset();
	bool load();
	bool save();

	int difficulty_mult() {
		if (difficulty == EASY) {
			return 3;
		}
		else if (difficulty == NORMAL) {
			return 2;
		}
		return 1;
	}

	Configuration();
private:
};

extern Configuration cfg;

#endif

#ifndef SETTINGS_LOOP_H
#define SETTINGS_LOOP_H

#include <allegro5/allegro.h>

#include <vector>

#include "loop.h"
#include "wrap.h"
#include "general.h"
#include "widgets.h"

class Settings_Loop : public Loop {
public:
	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	Settings_Loop();
	virtual ~Settings_Loop();

private:

	W_Button *video_button;
	W_Button *keyboard_button;
	W_Button *gamepad_button;
	W_Button *return_button;
};

#endif // SETTINGS_LOOP_H

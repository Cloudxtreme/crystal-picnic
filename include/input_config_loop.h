#ifndef INPUT_CONFIG_LOOP_H
#define INPUT_CONFIG_LOOP_H

#include <allegro5/allegro.h>

#include <vector>

#include "loop.h"
#include "wrap.h"
#include "general.h"
#include "widgets.h"

class Input_Config_Loop : public Loop {
public:
	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	Input_Config_Loop(bool keyboard);
	virtual ~Input_Config_Loop();

private:

	bool keyboard;

	int getting_key;
	int getting_button;

	W_Button *buttons[12];
	W_Button *defaults_button;
	W_Button *done_button;

	int keys[11];
	int gamepad_buttons[12];

	std::vector<std::string> labels;
};

extern bool dont_process_dpad_events;

#endif // INPUT_CONFIG_LOOP_H

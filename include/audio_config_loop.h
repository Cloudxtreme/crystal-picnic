#ifndef AUDIO_CONFIG_LOOP_H
#define AUDIO_CONFIG_LOOP_H

#include <tgui2_widgets.hpp>

#include "loop.h"
#include "widgets.h"

class Audio_Config_Loop : public Loop {
public:
	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	Audio_Config_Loop();
	virtual ~Audio_Config_Loop();

private:

	W_Slider *sfx_slider;
	W_Slider *music_slider;
	W_Checkbox *reverb_checkbox;
	W_Audio_Settings_Button *save_button;
};

#endif // AUDIO_CONFIG_LOOP_H

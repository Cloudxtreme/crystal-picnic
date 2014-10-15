#ifndef LANGUAGE_CONFIG_LOOP_H
#define LANGUAGE_CONFIG_LOOP_H

#include <allegro5/allegro.h>

#include <tgui2_widgets.hpp>

#include <vector>

#include "loop.h"
#include "wrap.h"
#include "general.h"
#include "widgets.h"

class Language_Config_Loop : public Loop {
public:
	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	Language_Config_Loop();
	virtual ~Language_Config_Loop();

private:

	W_Scrolling_List *language_list;
	W_Button *save_button;
	bool list_was_activated;
	std::vector<std::string> untranslated_languages;
};

#endif // LANGUAGE_CONFIG_LOOP_H

#ifndef DIFFICULTY_LOOP_H
#define DIFFICULTY_LOOP_H

#include <allegro5/allegro.h>

#include <vector>

#include "loop.h"
#include "wrap.h"
#include "general.h"
#include "widgets.h"

class Difficulty_Loop : public Loop {
public:
	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	Difficulty_Loop();
	virtual ~Difficulty_Loop();

private:

	W_Translated_Button *easy_button;
	W_Translated_Button *normal_button;
	W_Translated_Button *hard_button;
};

#endif // DIFFICULTY_LOOP_H

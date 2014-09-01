#ifndef SAVELOAD_LOOP_H
#define SAVELOAD_LOOP_H

#include <allegro5/allegro.h>

#include "loop.h"
#include "widgets.h"

class SaveLoad_Loop : public Loop {
public:
	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	SaveLoad_Loop(bool saving);
	virtual ~SaveLoad_Loop();

private:
	bool saving;

	W_SaveLoad_Button *slots[3];
	W_Button *return_button;
};

#endif // SAVELOAD_LOOP_H

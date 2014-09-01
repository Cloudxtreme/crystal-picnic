#ifndef HQM_LOOP_H
#define HQM_LOOP_H

#include <allegro5/allegro.h>

#include "loop.h"
#include "widgets.h"

class HQM_Loop : public Loop {
public:
	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	HQM_Loop();
	virtual ~HQM_Loop();

private:

	int max_w;

	W_Button *go_button;
	W_Button *pause_button;
	W_Button *delete_button;
	W_Button *return_button;
};

#endif // HQM_LOOP_H

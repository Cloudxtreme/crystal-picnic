#ifndef CREDITS_LOOP_H
#define CREDITS_LOOP_H

#include <allegro5/allegro.h>

#include <vector>

#include "loop.h"
#include "wrap.h"
#include "general.h"

class Credits_Loop : public Loop {
public:
	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	Credits_Loop();
	virtual ~Credits_Loop();

private:

	int draw_bitmap(int index, int y);
	int draw_strings(int index, int y);

	std::vector<Wrap::Bitmap *> bitmaps;
	std::vector< std::vector<std::string > > strings;

	float offset;
	float total_offset;
};

#endif // CREDITS_LOOP_H

#ifndef MAIN_H
#define MAIN_H

#include <allegro5/allegro.h>
#include "engine.h"

bool try_purchase(ALLEGRO_EVENT_QUEUE *event_queue);

class Main
{
public:
	Main();
	~Main();

	bool init();
	void execute();
	void shutdown();
};

#endif

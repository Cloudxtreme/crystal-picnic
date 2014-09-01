#ifndef LOOP_H
#define LOOP_H

#include <allegro5/allegro.h>

class Loop {
public:
	/* NOTE: Each loop should guard against being inited twice! */
	virtual bool init() { inited = true; return true; }
	virtual void top() {}
	// return "handled"
	virtual bool handle_event(ALLEGRO_EVENT *event) { return false; }
	virtual bool logic() { return false; }
	virtual void draw() {}
	virtual void post_draw() {}
	virtual void return_to_loop() {}
	virtual void destroy_graphics() {}
	virtual void reload_graphics() {}
	Loop() { inited = false; }
	virtual ~Loop() {}

public:
	bool inited;
};

#define GET_AREA_LOOP General::find_in_vector<Area_Loop *, Loop *>(engine->get_loops())
#define GET_BATTLE_LOOP General::find_in_vector<Battle_Loop *, Loop *>(engine->get_loops())
#define GET_SPEECH_LOOP General::find_in_vector<Speech_Loop *, Loop *>(engine->get_loops())
#define GET_MENU_LOOP General::find_in_vector<Menu_Loop *, Loop *>(engine->get_loops())

#endif // LOOP_H

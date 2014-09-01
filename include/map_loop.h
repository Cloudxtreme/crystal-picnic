#ifndef MAP_LOOP_H
#define MAP_LOOP_H

#include <allegro5/allegro.h>

#include <string>

#include <wrap.h>

#include "loop.h"
#include "general.h"
#include "player.h"

class Map_Loop : public Loop {
public:
	struct Location {
		std::string name;
		General::Point<int> pos;
		int left, right, up, down;
	};

	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();
	
	void add_location(std::string name, General::Point<int> pos);
	void set_location_neighbors(std::string name, std::string left, std::string right, std::string up, std::string down);

	std::string get_current_location_name();

	std::vector<Player *> get_players();
	
	Map_Loop(std::string start_place);
	virtual ~Map_Loop();

private:
	General::Point<int> get_desired_position(int loc);
	int name_to_int(std::string name);
	void create_temporary_players();

	lua_State *lua_state;
	Wrap::Bitmap *map_bmp;
	std::vector<Location> locations;
	General::Point<float> offset;
	General::Point<float> desired_offset;
	std::string start_place;
	unsigned int curr_loc;

	bool need_joy_axis_release;

	bool faded_in;

	Wrap::Bitmap *x_bmp;
	Wrap::Bitmap *node_bmp;

	std::vector<Player *> players;
};

#endif // MAP_LOOP_H

#ifndef ASTAR_CHARACTER_ROLE_H
#define ASTAR_CHARACTER_ROLE_H

#include "map_entity.h"
#include "general.h"
#include "astar.h"
#include "engine.h"
#include "character_role.h"

class AStar_Character_Role : public Character_Role, public A_Star::AStar_Callback_Interface {
public:
	void set_destination(General::Point<float> dest, bool run);
	bool is_following_path();

	void set_kamikaze(bool kamikaze);

	// AStar_Callback_Interface
	void astar_callback(std::list<A_Star::Node> *list_nodes);

	void update(Area_Manager *area);
	void draw(Area_Manager *area);

	void set_doesnt_wait_after_collision(bool doesnt_wait);

	AStar_Character_Role(Character_Map_Entity *character);
	virtual ~AStar_Character_Role();

protected:
	void destroy_astar_stuff();

	// A* stuff
	General::Point<float> astar_dest;
	A_Star::A_Star_Info *astar_info;
	std::list<A_Star::Node> *astar_nodes;
	std::list<A_Star::Node>::iterator astar_intermediate_node;
	General::Point<float> astar_intermediate_point;
	bool solid_with_area_save;

	double callback_called_time;

	bool running;
	double wait_after_collision;

	bool is_kamikaze;

	bool doesnt_wait_after_collision;
};

#endif // ASTAR_CHARACTER_ROLE_H

#ifndef FOLLOW_CHARACTER_ROLE_H
#define FOLLOW_CHARACTER_ROLE_H

#include "map_entity.h"
#include "general.h"
#include "astar.h"
#include "engine.h"
#include "character_role.h"

class Follow_Character_Role : public Character_Role {
public:
	void update(Area_Manager *area);
	void draw(Area_Manager *area);

	void reset();

	void set_actors(Character_Map_Entity *character, Map_Entity *target);

	Follow_Character_Role(Character_Map_Entity *character, Map_Entity *target);
	virtual ~Follow_Character_Role();

protected:
	Map_Entity *target;
	std::vector< std::pair< General::Point<float>, General::Point<float> > > tracking;
};

#endif // FOLLOW_CHARACTER_ROLE_H

#ifndef CHARACTER_MAP_ENTITY_H
#define CHARACTER_MAP_ENTITY_H

#include "general_types.h"
#include "map_entity.h"

class Character_Role;

class Character_Map_Entity : public Map_Entity {
public:
	Character_Role *get_role();
	void set_role(Character_Role *role);
	
	void character_set_position(General::Point<float> pos);
	void character_logic();
	void character_draw();

	void kamikaze(float x, float y);
	
	Character_Map_Entity(std::string name);
	virtual ~Character_Map_Entity();
protected:
	Character_Role *role;
	bool home_set;
	bool do_kamikaze;
	General::Point<float> kamikaze_point;
};

#endif

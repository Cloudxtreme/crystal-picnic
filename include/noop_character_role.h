#ifndef NOOP_CHARACTER_ROLE_H
#define NOOP_CHARACTER_ROLE_H

#include "map_entity.h"
#include "character_role.h"

class NoOp_Character_Role : public Character_Role {
public:
	void update(Area_Manager *area) {}
	void draw(Area_Manager *area) {}
	NoOp_Character_Role(Character_Map_Entity *e) : Character_Role(e) {}
	virtual ~NoOp_Character_Role() {}

protected:
};

#endif // NOOP_CHARACTER_ROLE_H

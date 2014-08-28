#ifndef CHARACTER_ROLE_H
#define CHARACTER_ROLE_H

#include "map_entity.h"
#include "general.h"
#include "engine.h"

class Character_Map_Entity;

class Character_Role {
public:
	virtual void update(Area_Manager *area) = 0;
	virtual void draw(Area_Manager *area) = 0;

	Character_Role(Character_Map_Entity *character);
	virtual ~Character_Role() {};

protected:
	Character_Map_Entity *entity;
};

#endif // _CHARACTER_ROLE_H

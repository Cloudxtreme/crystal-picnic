#include "weaponized_entity.h"
#include "general.h"

#include <string>

bool Weaponized_Entity::load_weapon_animations(std::string weapon_path, std::string xml_path)
{
	weapon_anim_set = new Animation_Set();
	bool success = weapon_anim_set->load(weapon_path, xml_path);
	if (!success) {
		delete weapon_anim_set;
		weapon_anim_set = NULL;
		return false;
	}

	return true;
}

Weaponized_Entity::Weaponized_Entity() :
	weapon_anim_set(NULL)
{
}

Weaponized_Entity::~Weaponized_Entity()
{
}

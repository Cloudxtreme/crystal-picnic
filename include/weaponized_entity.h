#ifndef WEAPONIZED_ENTITY_H
#define WEAPONIZED_ENTITY_H

#include "animation_set.h"

class Weaponized_Entity {
public:
	Animation_Set *get_weapon_animation_set() { return weapon_anim_set; }
	void set_weapon_animation_set(Animation_Set *a) { weapon_anim_set = a; }
	bool load_weapon_animations(std::string weapon_path, std::string xml_path);

	Weaponized_Entity();
	~Weaponized_Entity();

protected:
	Animation_Set *weapon_anim_set;
};

#endif // WEAPONIZED_ENTITY_H

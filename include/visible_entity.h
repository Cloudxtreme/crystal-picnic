#ifndef VISIBLE_ENTITY_H
#define VISIBLE_ENTITY_H

#include "animation_set.h"
#include "skeleton.h"

class Visible_Entity {
public:
	Animation_Set *get_animation_set() { return anim_set; }
	Skeleton::Skeleton *get_skeleton() { return skeleton; } // path to anim_set folder or just name.xml for skeletons -- checks for skeleton first

	virtual ~Visible_Entity() {}

protected:
	Animation_Set *anim_set;
	Skeleton::Skeleton *skeleton;
};

#endif // VISIBLE_ENTITY_H

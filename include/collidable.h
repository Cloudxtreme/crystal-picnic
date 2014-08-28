#ifndef COLLIDABLE_H
#define COLLIDABLE_H

#include "triangulate.h"
#include "general_types.h"

namespace Bones {
	struct Bone;
}

enum Collidable_Type {
	COLLIDABLE_POINT,
	COLLIDABLE_BOX,
	COLLIDABLE_BONES
};

class Collidable {
public:
	virtual Collidable_Type collidable_get_type() = 0;
	virtual void collidable_get_position(
		General::Point<float> &pos
	) = 0;
	virtual void collidable_get_box(
		General::Point<int> &offset,
		General::Size<int> &size
	) {
		(void)offset;
		(void)size;
	}
	virtual void collidable_get_bones(
		std::vector< Bones::Bone > &bones
	) {
		(void)bones;
	}

	bool collides_with(Collidable &c);

	bool collides_with_point(
		General::Point<float> pos
	);
	bool collides_with_box(
		General::Point<float> pos,
		General::Point<int> offset,
		General::Size<int> size
	);
	bool collides_with_bones(
		General::Point<float> pos,
		std::vector< Bones::Bone > &bones
	);

	virtual ~Collidable() {}
};

#endif // COLLIDABLE_H

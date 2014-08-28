#ifndef POSITIONED_ENTITY_H
#define POSITIONED_ENTITY_H

#include "general_types.h"
#include "entity.h"

class Positioned_Entity : public Entity
{
public:
	virtual General::Point<float> get_position() {
		return pos;
	}
	virtual void set_position(General::Point<float> p) {
		pos = p;
	}

	float get_z() { return z; }
	void set_z(float z) { this->z = z; }

	Positioned_Entity() {
		z = 0.0f;
	}
	
	virtual ~Positioned_Entity() {}

protected:
	General::Point<float> pos;
	float z;
};

#endif // POSITIONED_ENTITY_H


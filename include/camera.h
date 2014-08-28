#ifndef CAMERA_H
#define CAMERA_H

#include "general.h"

class Camera {
public:
	virtual General::Point<float> get_camera_pos() = 0;
	virtual bool move_camera(General::Point<float> delta, General::Point<float> entity_pos, bool tracking_entity) = 0;

	Camera();
};

#endif // CAMERA_H

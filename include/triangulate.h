#ifndef TRIANGULATE_H
#define TRIANGULATE_H

#include <cmath>
#include <climits>

#include <list>
#include <vector>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

#include "general_types.h"

namespace Triangulate {

struct Triangle {
	General::Point<float> points[3];

	Triangle() {}
	Triangle(const Triangle &rhs) {
		for (int i = 0; i < 3; i++) {
			points[i] = rhs.points[i];
		}
	}
	Triangle &operator=(const Triangle &rhs) {
		for (int i = 0; i < 3; i++) {
			points[i] = rhs.points[i];
		}

		return *this;
	}
};
	
void get_triangles(std::vector< General::Point<float> > points, std::vector<int> splits, std::vector<Triangle> &ret_triangles);

} // end namespace

#endif // EAR_CLIP_H

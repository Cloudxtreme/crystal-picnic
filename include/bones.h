#ifndef BONES_H
#define BONES_H

#include <string>
#include <map>

#include "triangulate.h"
#include "animation_set.h"

namespace Bones {

enum Type {
	BONE_NORMAL = 0, // can take damage
	BONE_ATTACK, // deals damage
	BONE_RESISTANT, // takes no damage
	BONE_WEAK // takes lots of damage
};

struct Bone {
	Type type;

	Bone(
			Type type,
			std::vector< General::Point<float> > &outline,
			std::vector<Triangulate::Triangle> &triangles,
			General::Size<int> size
		) :
		size(size)
	{
		this->type = type;
	
		std::vector< General::Point<float> >::iterator it2;

		for (it2 = outline.begin(); it2 != outline.end(); it2++) {
			General::Point<float> p = *it2;
			this->outline.push_back(p);
			mirrored_outline.push_back(General::Point<float>(
				-p.x,
				p.y
			));
		}

		std::vector<Triangulate::Triangle>::iterator it;

		for (it = triangles.begin(); it != triangles.end(); it++) {
			this->triangles.push_back(*it);
			Triangulate::Triangle &t = *it;
			Triangulate::Triangle t2;
			for (int i = 0; i < 3; i++) {
				t2.points[i].x = -t.points[i].x;
				t2.points[i].y = t.points[i].y;
			}
			mirrored_triangles.push_back(t2);
		}
	}

	Bone(const Bone &rhs) {
		type = rhs.type;
		outline = rhs.outline;
		mirrored_outline = rhs.mirrored_outline;
		triangles = rhs.triangles;
		mirrored_triangles = rhs.mirrored_triangles;
		size = rhs.size;
	}
	Bone &operator=(const Bone &rhs) {
		type = rhs.type;
		outline = rhs.outline;
		mirrored_outline = rhs.outline;
		triangles = rhs.triangles;
		mirrored_triangles = rhs.mirrored_triangles;
		size = rhs.size;

		return *this;
	}

	std::vector<Triangulate::Triangle> &get() {
		return triangles;
	}
	std::vector<Triangulate::Triangle> &get_mirrored() {
		return mirrored_triangles;
	}
	std::vector< General::Point<float> > &get_outline() {
		return outline;
	}
	std::vector< General::Point<float> > &get_outline_mirrored() {
		return mirrored_outline;
	}

	General::Size<int> get_size() { return size; }
	
	General::Size<float> get_extents() const;

protected:
	std::vector< General::Point<float> > outline;
	std::vector< General::Point<float> > mirrored_outline;
	std::vector<Triangulate::Triangle> triangles;
	std::vector<Triangulate::Triangle> mirrored_triangles;
	General::Size<int> size;
};

void load(
	Animation_Set *anim_set,
	std::string info_filename,
	std::string path_to_files,
	std::map< std::pair<std::string, int>, std::vector<Bone> > &bones
);
void load(std::vector<Bone> &bones, int bmp_w, int bmp_h, std::string filename);

}

#endif // _BONES_H

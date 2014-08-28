#include "crystalpicnic.h"
#include "triangulate.h"
#include "collision_detection.h"

#include <poly2tri/poly2tri.h>

/* This used to be a full triangulator, but Allegro has one now so we use it */

namespace Triangulate {

/*
struct DATA {
	float *vertices;
	std::vector<Triangle> *triangle_vec;
};

void emit_triangle(int i1, int i2, int i3, void *_data)
{
	DATA *data = (DATA *)_data;
	
	General::Point<float> p1 = General::Point<float>(data->vertices[i1*2], data->vertices[i1*2+1]);
	General::Point<float> p2 = General::Point<float>(data->vertices[i2*2], data->vertices[i2*2+1]);
	General::Point<float> p3 = General::Point<float>(data->vertices[i3*2], data->vertices[i3*2+1]);

	Triangle t;
	t.points[0] = p1;
	t.points[1] = p2;
	t.points[2] = p3;
	data->triangle_vec->push_back(t);
}
*/

void get_triangles(std::vector< General::Point<float> > point_vec, std::vector<int> split_vec, std::vector<Triangulate::Triangle> &triangle_vec)
{
	if (split_vec.size() == 0 || split_vec[0] <= 0) {
		return;
	}

	//float *vertices;
	//int *counts;

	// check for duplicated points and give a warning
	for (size_t i = 0; i < point_vec.size(); i++) {
		for (size_t j = i+1; j < point_vec.size(); j++) {
			if (point_vec[i] == point_vec[j]) {
				printf("***** ERROR: DUPLICATE POINT IN POLYGON *****\n");
				int *death = 0;
				*death = 0xdead;
			}
		}
	}

	std::vector< std::vector<p2t::Point *> > all;
	int curr = 0;
	
	for (size_t i = 0; i < split_vec.size(); i++) {
		all.push_back(std::vector<p2t::Point *>());
		for (; curr < split_vec[i]; curr++) {
			p2t::Point *p = new p2t::Point();
			p->x = point_vec[curr].x;
			p->y = point_vec[curr].y;
			all[i].push_back(p);
		}
	}

	p2t::CDT *cdt = new p2t::CDT(all[0]);
	for (size_t i = 1; i < all.size(); i++) {
		cdt->AddHole(all[i]);
	}

	cdt->Triangulate();

	std::vector<p2t::Triangle *> tris = cdt->GetTriangles();

	for (size_t i = 0; i < tris.size(); i++) {
		Triangulate::Triangle t;
		p2t::Point *p;
		for (int j = 0; j < 3; j++) {
			p = tris[i]->GetPoint(j);
			t.points[j].x = p->x;
			t.points[j].y = p->y;
		}
		triangle_vec.push_back(t);
	}

	for (size_t i = 0; i < all.size(); i++) {
		for (size_t j = 0; j < all[i].size(); j++) {
			delete all[i][j];
		}
	}

	delete cdt;
}

} // End namespace

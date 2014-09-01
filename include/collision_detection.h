#ifndef COLLISION_DETECTION_H
#define COLLISION_DETECTION_H

#include <vector>
#include <list>

#include "general_types.h"
#include "triangulate.h"

bool checkcoll_point_point(General::Point<float> a, General::Point<float> b);
bool checkcoll_point_box(
		General::Point<float> point,
		General::Point<float> topleft,
		General::Point<float> bottomright
	);
bool checkcoll_point_circle(
		General::Point<float> point,
		General::Point<float> center,
		float radius
	);
bool checkcoll_point_polygon(
		General::Point<float> p1,
		std::vector< General::Point<float> > &poly,
		General::Point<float> poly_offset,
		int start_compare,
		int end_compare
	);
bool checkcoll_line_box(
	General::Point<float> a,
	General::Point<float> b,
	General::Point<float> topleft,
	General::Point<float> bottomright);
bool checkcoll_box_box(
		General::Point<float> topleft1,
		General::Point<float> bottomright1,
		General::Point<float> topleft2,
		General::Point<float> bottomright2
	);
bool checkcoll_box_circle(
	General::Point<float> topleft, General::Point<float> topright,
	General::Point<float> center, float radius);
bool checkcoll_box_polygon(
		General::Point<float> topleft,
		General::Point<float> bottomright,
		std::vector< General::Point<float> > poly,
		General::Point<float> poly_offset,
		General::Line<float> *ret_line
	);
bool checkcoll_circle_circle(
		General::Point<float> center1, float radius1,
		General::Point<float> center2, float radius2
	);
bool checkcoll_circle_polygon(
		General::Point<float> center, float radius,
		std::vector< General::Point<float> > poly,
		General::Point<float> poly_pos
	);
bool checkcoll_polygon_polygon(
		std::vector< General::Point<float> > &a,
			General::Point<float> topleft1,
		std::vector< General::Point<float> > &b,
			General::Point<float> topleft2
	);
bool checkcoll_line_line(
	const General::Point<float> *a1,
	const General::Point<float> *a2,
	const General::Point<float> *a3,
	const General::Point<float> *a4,
	General::Point<float> *result);
bool checkcoll_point_triangle(General::Point<float> *p, General::Point<float> *a, General::Point<float> *b, General::Point<float> *c);
float dist_point_to_box(General::Point<float> point, General::Point<float> topleft, General::Point<float> bottomright);
float dist_point_line_result(General::Point<float> point, General::Point<float> a, General::Point<float> b, General::Point<float> *result);
float dist_point_line(General::Point<float> point, General::Point<float> a, General::Point<float> b);
bool checkcoll_line_polygon(General::Point<float> p1, General::Point<float> p2, std::vector< General::Point<float> > polygon, General::Point<float> poly_offset);

#endif

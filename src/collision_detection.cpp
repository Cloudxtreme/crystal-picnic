#include <cmath>

#include "collision_detection.h"
#include "general.h"

bool checkcoll_point_point(General::Point<float> a, General::Point<float> b)
{
	return a.x == b.x && a.y == b.y;
}

bool checkcoll_point_box(
		General::Point<float> point,
		General::Point<float> topleft,
		General::Point<float> bottomright
	)
{
	return point.x >= topleft.x && point.x <= bottomright.x && point.y >= topleft.y && point.y <= bottomright.y;
}

bool checkcoll_point_circle(
		General::Point<float> point,
		General::Point<float> center,
		float radius
	)
{
	float r2 = radius*radius;
	return (pow(point.x-center.x, 2) + pow(point.y-center.y, 2)) <= r2;
}

/*
bool checkcoll_point_polygon(
		General::Point<float> p1,
		std::vector< General::Point<float> > &poly,
		General::Point<float> poly_offset,
		int start_compare,
		int end_compare
	)
{
	int count = 0;
		
	General::Point<float> p2 = General::Point<float>(
		999999, p1.y
	);

	for (int i = start_compare; i < end_compare; i++) {
		int j = (i == end_compare-1) ? start_compare : i+1;
		General::Point<float> p3 = General::Point<float>(
			poly[i].x + poly_offset.x,
			poly[i].y + poly_offset.y
		);
		General::Point<float> p4 = General::Point<float>(
			poly[j].x + poly_offset.x,
			poly[j].y + poly_offset.y
		);
		General::Point<float> result;
		if (checkcoll_line_line(&p1, &p2, &p3, &p4, &result)) {
			count++;
			if (General::distance(p4.x, p4.y, result.x, result.y) < 1) {
				// Skip one if it collides on an endpoint because it will definitely collide the next time
				// and we don't want to add that
				i++;
			}
		}
	}

	if (count % 2 == 1) return true;
	return false;
}
*/

bool checkcoll_point_polygon(
		General::Point<float> p,
		std::vector< General::Point<float> > &in_poly,
		General::Point<float> poly_offset,
		int start_compare,
		int end_compare
	)
{
	int i, j, c = 0;
	for (i = start_compare, j = end_compare-1; i < end_compare; j = i++) {
		if (((in_poly[i].y+poly_offset.y > p.y) != (in_poly[j].y+poly_offset.y > p.y)) &&
				(p.x < ((in_poly[j].x+poly_offset.x)-(in_poly[i].x+poly_offset.x)) *
				(p.y-(in_poly[i].y+poly_offset.y)) / ((in_poly[j].y+poly_offset.y)-(in_poly[i].y+poly_offset.y)) +
				(in_poly[i].x+poly_offset.x)))
			c = !c;
	}
	return c;
}

bool checkcoll_line_box(
	General::Point<float> a,
	General::Point<float> b,
	General::Point<float> topleft,
	General::Point<float> bottomright)
{
	General::Point<float> topright(
		bottomright.x,
		topleft.y
	);
	General::Point<float> bottomleft(
		topleft.x,
		bottomright.y
	);

	if (
		checkcoll_line_line(&a, &b, &topleft, &topright, NULL) ||
		checkcoll_line_line(&a, &b, &topright, &bottomright, NULL) ||
		checkcoll_line_line(&a, &b, &bottomright, &bottomleft, NULL) ||
		checkcoll_line_line(&a, &b, &bottomleft, &topleft, NULL))
	{
		return true;
	}
	return false;
}

bool checkcoll_box_box(
		General::Point<float> topleft1,
		General::Point<float> bottomright1,
		General::Point<float> topleft2,
		General::Point<float> bottomright2
	)
{
	if (
		topleft2.x > bottomright1.x ||
		bottomright2.x < topleft1.x ||
		topleft2.y > bottomright1.y ||
		bottomright2.y < topleft1.y
	) {
		return false;
	}
	return true;
}

bool checkcoll_box_circle(
	General::Point<float> topleft, General::Point<float> topright,
	General::Point<float> center, float radius)
{
	if (checkcoll_point_box(center, topleft, topright)) {
		return true;
	}

	float d = dist_point_to_box(center, topleft, topright);
	
	return d <= radius;
}

// FIXME: doesn't support holes in polygon
bool checkcoll_box_polygon(
		General::Point<float> topleft,
		General::Point<float> bottomright,
		std::vector< General::Point<float> > poly,
		General::Point<float> poly_offset,
		General::Line<float> *ret_line
	)
{
	/* Check box against poly then poly against box */

	if (
		checkcoll_point_polygon(topleft, poly, poly_offset, 0, poly.size()) ||
		checkcoll_point_polygon(General::Point<float>(bottomright.x, topleft.y), poly, poly_offset, 0, poly.size()) ||
		checkcoll_point_polygon(bottomright, poly, poly_offset, 0, poly.size()) ||
		checkcoll_point_polygon(General::Point<float>(topleft.x, bottomright.y), poly, poly_offset, 0, poly.size())
	) {
		return true;
	}

	for (size_t i = 0; i < poly.size(); i++) {
		General::Point<float> p(poly[i].x+poly_offset.x, poly[i].y+poly_offset.y);
		if (checkcoll_point_box(p, topleft, bottomright)) {
			return true;
		}
	}

	return false;
}

bool checkcoll_circle_circle(
		General::Point<float> center1, float radius1,
		General::Point<float> center2, float radius2
	)
{
	float dx = center1.x - center2.x;
	float dy = center1.y - center2.y;
	float dist = dx*dx + dy*dy;
	return dist < pow(radius1 + radius2, 2);
}

bool checkcoll_circle_polygon(
		General::Point<float> center, float radius,
		std::vector< General::Point<float> > poly,
		General::Point<float> poly_pos
	)
{
	if (checkcoll_point_polygon(center, poly, poly_pos, 0, poly.size())) {
		return true;
	}

	for (size_t i = 0; i < poly.size(); i++) {
		General::Point<float> p1 = General::Point<float>(
			poly[i].x + poly_pos.x,
			poly[i].y + poly_pos.y
		);
		General::Point<float> p2 = General::Point<float>(
			poly[(i+1)%poly.size()].x + poly_pos.x,
			poly[(i+1)%poly.size()].y + poly_pos.y
		);
		if (dist_point_line(center, p1, p2) < radius) {
			return true;
		}
	}

	return false;
}

bool checkcoll_polygon_polygon(
		std::vector< General::Point<float> > &a,
			General::Point<float> topleft1,
		std::vector< General::Point<float> > &b,
			General::Point<float> topleft2
	)
{
	for (size_t i = 0; i < a.size(); i++) {
		if (checkcoll_point_polygon(
				General::Point<float>(a[i].x+topleft1.x, a[i].y+topleft1.y),
				b,
				topleft2,
				0,
				b.size()))
		{
			return true;
		}
	}
	for (size_t i = 0; i < b.size(); i++) {
		if (checkcoll_point_polygon(
				General::Point<float>(b[i].x+topleft2.x, b[i].y+topleft2.y),
				a,
				topleft1,
				0,
				a.size()))
		{
			return true;
		}
	}

	return false;
}


/* Miscellaneous collision detection: */


// NOTE: code found here: http://forums.create.msdn.com/forums/t/280.aspx
bool checkcoll_line_line(
	const General::Point<float> *a1,
	const General::Point<float> *a2,
	const General::Point<float> *a3,
	const General::Point<float> *a4,
	General::Point<float> *result)
{
	double Ua, Ub;

	Ua = ((a4->x - a3->x) * (a1->y - a3->y) - (a4->y - a3->y) * (a1->x - a3->x)) / ((a4->y - a3->y) * (a2->x - a1->x) - (a4->x - a3->x) * (a2->y - a1->y));

	Ub = ((a2->x - a1->x) * (a1->y - a3->y) - (a2->y - a1->y) * (a1->x - a3->x)) / ((a4->y - a3->y) * (a2->x - a1->x) - (a4->x - a3->x) * (a2->y - a1->y));

	if (Ua >= 0.0f && Ua <= 1.0f && Ub >= 0.0f && Ub <= 1.0f)
	{
		if (result) {
			result->x = a1->x + Ua * (a2->x - a1->x);
			result->y = a1->y + Ua * (a2->y - a1->y);
		}

		return true;
	}
	else
	{
		return false;
	}
}

static double cross_product(double a, double b, double c, double d)
{
	return a*d - b*c;
}

static bool same_side(General::Point<float> *p1, General::Point<float> *p2, General::Point<float> *a, General::Point<float> *b)
{
	double cp1 = cross_product(b->x-a->x, b->y-a->y, p1->x-a->x, p1->y-a->y);
	double cp2 = cross_product(b->x-a->x, b->y-a->y, p2->x-a->x, p2->y-a->y);
	if (cp1*cp2 > 0) {
		return true;
	}
	return false;
}

bool checkcoll_point_triangle(General::Point<float> *p, General::Point<float> *a, General::Point<float> *b, General::Point<float> *c)
{
	if (same_side(p,a, b,c) && same_side(p,b, a,c) && same_side(p,c, a,b)) {
		return true;
	}
	return false;
}

// helper function: does not account for point INSIDE box (used for box circle detection above)
float dist_point_to_box(General::Point<float> point, General::Point<float> topleft, General::Point<float> bottomright)
{
	float xdist[2], ydist[2];

	xdist[0] = point.x - topleft.x;
	xdist[1] = point.x - bottomright.x;
	ydist[0] = point.y - topleft.y;
	ydist[1] = point.y - bottomright.y;

	int minx = fabs(xdist[0]) < fabs(xdist[1]) ? 0 : 1;
	int miny = fabs(ydist[0]) < fabs(ydist[1]) ? 0 : 1;

	return sqrt((float)(minx*minx + miny*miny));
}

float dist_point_line_result(General::Point<float> point, General::Point<float> a, General::Point<float> b, General::Point<float> *result)
{
	// above function checks distance to end points of line first, so that's taken care of
	// and we're guaranteed there's a perpendicular line crossing the line segment

	// find distance by finding nearest intersection (perpendicular line from point to a,b)
	// find slope of line
	float run = b.y - a.y;

	// don't divide by zero
	if (run == 0) {
		return dist_point_to_box(point, a, b);
	}

	float rise = b.x - a.x;
	float slope = rise / run;

	float neg_slope = -slope;
	float neg_run = FLT_MAX / 3; // really big number
	float neg_rise = neg_slope * neg_run;

	General::Point<float> c = General::Point<float>(
		point.x - neg_run,
		point.y - neg_rise
	);
	General::Point<float> d = General::Point<float>(
		point.x + neg_run,
		point.y + neg_rise
	);

	if (checkcoll_line_line(&a, &b, &c, &d, result)) {
		float dx = point.x - result->x;
		float dy = point.y - result->y;
		return sqrt(dx*dx + dy*dy);
	}

	// this may not ever happen
	return FLT_MAX;
}

float dist_point_line(General::Point<float> point, General::Point<float> a, General::Point<float> b)
{
	General::Point<float> result;
	return dist_point_line_result(point, a, b, &result);
}

bool checkcoll_line_polygon(General::Point<float> p1, General::Point<float> p2, std::vector< General::Point<float> > polygon, General::Point<float> poly_offset)
{
	for (size_t i = 0; i < polygon.size(); i++) {
		int j = (i+1) % polygon.size();
		General::Point<float> p3 (polygon[i].x+poly_offset.x, polygon[i].y+poly_offset.y);
		General::Point<float> p4 (polygon[j].x+poly_offset.x, polygon[j].y+poly_offset.y);
//		printf("checking %f,%f %f,%f -> %f,%f %f,%f\n",
//			p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
		if (checkcoll_line_line(&p1, &p2, &p3, &p4, NULL)) {
			return true;
		}
	}

	return false;
}


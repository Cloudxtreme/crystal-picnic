// Uncomment this to use Allegro to display the results (requires extra linker options)
#define HAVE_ALLEGRO

// Uncomment this to draw to a large offscreen bitmap and save it for precise
// viewing. Requires HAVE_ALLEGRO.
//#define BIG
#define SCALE 3.0

#include <cmath>
#include <climits>

#include <list>
#include <vector>

#ifdef HAVE_ALLEGRO
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#endif

// WARNING: Not working
//#define STRICT

#include "dllist.h"

#include "testing.h"

struct Poly_Point {
	float x, y;
	bool is_ear_tip;
};

struct Triangle {
	Poly_Point pts[3];
};

static void print_points(std::vector<float> &points, int num)
{
	printf("%d points:\n", num/2);
	for (int i = 0; i < num; i+= 2) {
		printf("\t%g, %g,\n", points[i], points[i+1]);
	}
}

#ifdef HAVE_ALLEGRO
/*
static void draw_tri(Triangle *t, ALLEGRO_COLOR color)
{
	al_draw_line(t->pts[0].x, t->pts[0].y, t->pts[1].x, t->pts[1].y, color, 1);
	al_draw_line(t->pts[1].x, t->pts[1].y, t->pts[2].x, t->pts[2].y, color, 1);
	al_draw_line(t->pts[2].x, t->pts[2].y, t->pts[0].x, t->pts[0].y, color, 1);
}
*/
#endif

static inline float cross_product(float a, float b, float c, float d)
{
	return a*d - b*c;
}

static inline bool same_side(Poly_Point *p1, Poly_Point *p2, Poly_Point *a, Poly_Point *b)
{
	float cp1 = cross_product(b->x-a->x, b->y-a->y, p1->x-a->x, p1->y-a->y);
	float cp2 = cross_product(b->x-a->x, b->y-a->y, p2->x-a->x, p2->y-a->y);
	if (cp1*cp2 > 0) {
		return true;
	}
	return false;
}

static inline bool point_in_triangle(Poly_Point *p, Poly_Point *a, Poly_Point *b, Poly_Point *c)
{
#ifdef STRICT
	if (p == a || p == b || p == c) {
#else
	if (
		(p->x == a->x && p->y == a->y) ||
		(p->x == b->x && p->y == b->y) ||
		(p->x == c->x && p->y == c->y)
	) {
#endif
		return false;
	}
	if (same_side(p,a, b,c) && same_side(p,b, a,c) && same_side(p,c, a,b)) {
		return true;
	}
	return false;
}

static inline bool lines_collide(Poly_Point *a1, Poly_Point *a2, Poly_Point *a3, Poly_Point *a4, Poly_Point *result)
{
	result->x = 0;
	result->y = 0;

#ifdef STRICT
	if (a1 == a3 || a1 == a4 || a2 == a3 || a2 == a4) {
#else
	if (
		(a1->x == a3->x && a1->y == a3->y) ||
		(a1->x == a4->x && a1->y == a4->y) ||
		(a2->x == a3->x && a2->y == a3->y) ||
		(a2->x == a4->x && a2->y == a4->y)
	) {
#endif
		return false;
	}

	float denom = ((a1->x - a2->x)*(a3->y - a4->y) - (a1->y - a2->y)*(a3->x - a4->x));

	if (denom == 0)
		return false;
	
	float cx = ((a1->x*a2->y - a1->y*a2->x)*(a3->x - a4->x) - (a1->x - a2->x)*(a3->x*a4->y - a3->y*a4->x)) / denom;
	float cy = ((a1->x*a2->y - a1->y*a2->x)*(a3->y - a4->y) - (a1->y - a2->y)*(a3->x*a4->y - a3->y*a4->x)) / denom;

	result->x = cx;
	result->y = cy;

	float x1, y1, x2, y2;

	x1 = a1->x < a2->x ? a1->x : a2->x;
	x2 = a1->x > a2->x ? a1->x : a2->x;
	y1 = a1->y < a2->y ? a1->y : a2->y;
	y2 = a1->y > a2->y ? a1->y : a2->y;
	if (cx > x2 || cx < x1 || cy > y2 || cy < y1) {
		return false;
	}
	
	x1 = a3->x < a4->x ? a3->x : a4->x;
	x2 = a3->x > a4->x ? a3->x : a4->x;
	y1 = a3->y < a4->y ? a3->y : a4->y;
	y2 = a3->y > a4->y ? a3->y : a4->y;
	if (cx > x2 || cx < x1 || cy > y2 || cy < y1) {
		return false;
	}
	
	return true;
}

static inline float tri_angle(float a, float b, float c, float d, float e, float f)
{
	float dx = c - a;
	float dy = d - b;
	float angle1 = atan2(dy, dx);
	dx = c - e;
	dy = d - f;
	float angle2 = atan2(dy, dx);
	float result = angle1 - angle2;
	if (result < -M_PI) result += M_PI*2;
	return result;
}

static inline bool is_ear(
	DL_List::List *list,
	DL_List::Node *pt
) {
	Poly_Point *tri[3] = {
		(Poly_Point *)pt->prev->data,
		(Poly_Point *)pt->data,
		(Poly_Point *)pt->next->data,
	};

	bool alert = false;
	if (tri[1]->x == 508 && tri[1]->y == 375) alert = true;
	
	float a = tri_angle(tri[0]->x, tri[0]->y, tri[1]->x, tri[1]->y, tri[2]->x, tri[2]->y);
	while (a < 0) a += M_PI*2;
	if (!(a > M_PI && a < M_PI*2)) {
		if (alert) printf("1\n");
		return false;
	}
	
	DL_List::Node *tmp = list->head;

	while (1) {
		Poly_Point *tmp_pt = (Poly_Point *)tmp->data;

		int i;
		for (i = 0; i < 3; i++) {
#ifdef STRICT
			if (tmp_pt == tri[i]) {
#else
			if (tmp_pt->x == tri[i]->x && tmp_pt->y == tri[i]->y) {
#endif
				break;
			}
		}

		if (i != 3) {
			goto loop;
		}

		if (point_in_triangle(tmp_pt, tri[0], tri[1], tri[2])) {
			if (alert) printf("2\n");
			return false;
		}

loop:
		tmp = tmp->next;
		if (tmp == list->head)
			break;
	}
	
	if (alert) printf("-0-\n");
	
	return true;
}

static inline DL_List::Node *find_ear(DL_List::List *list, void *data)
{
	DL_List::Node *node = list->head;
	if (node == 0)
		return 0;
	do {
		if (node->data == data) {
			return node;
		}
		node = node->next;
	} while (node != list->head);
	return 0;
}

// assumes there are no marked ears and ear list is empty
static void scan_for_ears(DL_List::List *vert_list, DL_List::List *ear_list, int &num_ears)
{
	num_ears = 0;
	DL_List::Node *pt = vert_list->head;

	for (;;) {
		if (is_ear(vert_list, pt)) {
			Poly_Point *p = (Poly_Point *)pt->data;
			al_draw_circle(p->x, p->y, 3, al_map_rgb(255, 255, 255), 1);
			DL_List::Node *node = new DL_List::Node;
			node->data = p;
			p->is_ear_tip = true;
			DL_List::add(ear_list, node);
			num_ears++;
		}
		pt = pt->next;
		if (pt == vert_list->head) break;
	}

	al_flip_display();
	al_rest(10);
}

void get_triangles(float *points_tmp, int *splits_tmp, int num_splits,
	std::list<Triangle *> &triangles
) {
	DL_List::List *vert_list = DL_List::create();
	DL_List::List *ear_list = DL_List::create();
	DL_List::Node *pt;

	std::vector<float> points;
	for (int i = 0; i < splits[num_splits-1]; i++) {
		points.push_back(points_tmp[i]);
	}
	int *splits = new int[num_splits];
	memcpy(splits, splits_tmp, sizeof(int)*num_splits);
	int hole_point_counts[num_splits-1];
	for (int i = 0; i < num_splits-1; i++) {
		hole_point_counts[i] = splits[i+1] - splits[i];
	}
	int num_values = splits[num_splits-1];
	int main_values = splits[0];
	int last_split_index = 0;

	for (int k = 0; k < num_splits; k++) {
		int i;
		if (k == 0)
			i = 0;
		else
			i = splits[k-1];
		for (; i < splits[k]; i += 2) {
			int j = i+2;
			if (j >= splits[k]) j = k == 0 ? 0 : splits[k-1];
			Poly_Point a, b;
			a.x = points[i];
			a.y = points[i+1];
			b.x = points[j];
			b.y = points[j+1];
			al_draw_line(a.x, a.y, b.x, b.y, al_map_rgb(0, 255, 0), 1);
		}
	}
	al_flip_display();
	al_rest(3);

	// make a split for holes
	int i;
	bool finished[num_splits-1];
	for (i = 0; i < num_splits-1; i++) {
		finished[i] = false;
	}
	for (i = 0; i < num_splits-1; i++) {
		// find rightmost unfinished hole
		int rightmost;
		float right_value = -10000;
		int rightmost_index;
		int hole_index_offset;
		for (int j = 0; j < num_splits-1; j++) {
			if (finished[j]) continue;
			// check each point
			for (int k = 0; k < hole_point_counts[j]; k += 2) {
				int index = splits[j]+k;
				float this_x = points[index];
				if (this_x > right_value) {
					right_value = this_x;
					rightmost = j;
					rightmost_index = index;
					hole_index_offset = k;
				}
			}
		}
		finished[rightmost] = true;
		int hole_num_values = hole_point_counts[rightmost];
		// find nearest edge directly right of rightmost point
		// in rightmost hole

		// use these two points as an intersection line to find
		// closest outer right edge
		Poly_Point hole_rightmost, hole_wayright;
		hole_rightmost.x = points[rightmost_index];
		hole_rightmost.y = points[rightmost_index+1];
		hole_wayright.x = 10000;
		hole_wayright.y = hole_rightmost.y;
		float minx = 10000;
		int best_points[2] = { -1, -1 };

		// the point at which we will make the split
		int main_right_index = -1;
		float main_point_x;
		Poly_Point main_point;

		// go through every line segment and find the
		// nearest-to-the-right one to the hole.
		for (int j = 0; j < main_values; j += 2) {
			int j2 = (j+2) % main_values;
			Poly_Point p1, p2;
			p1.x = points[j];
			p1.y = points[j+1];
			p2.x = points[j2];
			p2.y = points[j2+1];
			if ((hole_rightmost.x == p1.x && hole_rightmost.y == p1.y) ||
				//(hole_wayright.x == p2.x && hole_wayright.y == p2.y)) {
				(hole_rightmost.x == p2.x && hole_wayright.y == p2.y)) {
				continue;
			}
			Poly_Point result;
			if (lines_collide(&hole_rightmost, &hole_wayright, &p1, &p2, &result)) {
				if (result.x < minx) {
					minx = result.x;
					if (p1.y == hole_rightmost.y || p2.y == hole_rightmost.y) {
						if (p1.y > p2.y) {
							best_points[0] = j;
							best_points[1] = j2;
						}
						else {
							best_points[0] = j2;
							best_points[1] = j;
						}
					}
					else {
						if (p1.x > p2.x) {
							best_points[0] = j;
							best_points[1] = j2;
						}
						else {
							best_points[0] = j2;
							best_points[1] = j;
						}
					}
				}
			}
		}

		bool found = false;
		while (!found) {
			for (int j = 0; j < 2; j++) {
				Poly_Point best;
				best.x = points[best_points[j]];
				best.y = points[best_points[j]+1];
				for (int k = 0; k < main_values; k += 2) {
					int k2 = (k+2) % main_values;
					//printf("j=%d k=%d k2=%d\n", j, k, k2);
					Poly_Point pp1, pp2;
					pp1.x = points[k];
					pp1.y = points[k+1];
					pp2.x = points[k2];
					pp2.y = points[k2+1];
					if ((hole_rightmost.x == pp1.x && hole_rightmost.y == pp1.y) ||
						//(hole_wayright.x == pp2.x && hole_wayright.y == pp2.y)) {
						(hole_rightmost.x == pp2.x && hole_rightmost.y == pp2.y)) {
						continue;
					}
					if ((best.x == pp1.x && best.y == pp1.y) ||
						//(hole_wayright.x == pp2.x && hole_wayright.y == pp2.y)) {
						(best.x == pp2.x && best.y == pp2.y)) {
						continue;
					}
					Poly_Point result;
					if (lines_collide(&hole_rightmost, &best, &pp1, &pp2, &result)) {
						//printf("lines collide\n");
						//if (j == 0)
						//	continue;
						if (pp1.x > pp2.x) {
							best_points[0] = k;
							best_points[1] = k2;
						}
						else {
							best_points[0] = k2;
							best_points[1] = k;
						}
						goto new_set;
					}
				}
				printf("found\n");
				found = true;
				main_right_index = best_points[j];
				printf("It is %f %f\n", points[main_right_index], points[main_right_index+1]);
				main_point_x = best.x;
				main_point = best;
				break;
new_set:;
			}
			if (found)
				break;
		}
		printf("found=%d\n", found);

		// adjust splits
		for (int j = 0; j < num_splits; j++) {
			if (j == rightmost) {
				splits[j] = main_right_index+2;
			}
			else if (j < rightmost) {
				splits[j] += hole_num_values+4;
			}
			else
				splits[j] += 4;

		}
		last_split_index = rightmost;

		printf("main_right_index=%d\n", main_right_index);

		if (i > 0) {
			for (int j = 0; j < num_splits; j++) {
				if (main_right_index == splits[j]-2) {
					// HERE
					int pos = rightmost_index;
					if (points[main_right_index+3] > points[pos+1]) {
						bool found = false;
						int k;
						for (k = main_right_index+2; k < main_values; k += 2) {
							if (points[main_right_index] == points[k] && points[main_right_index+1] == points[k+1]) {
								found = true;
								break;
							}
						}
						if (found) {
							main_right_index = k;
							splits[j] = main_right_index + 2;
							break;
						}
					}
				}
			}
		}

		// create the split
		// points need to be re-arranged: hole points put inline with
		// outer edge points and two new points inserted
	
		// insert space for two extra points and the hole.
		// extra space will be removed later
		for (int j = 0; j < hole_num_values+4; j++) {
			points.insert(points.begin()+(main_right_index+2), 0);
		}
		// copy the hole in place, starting with the rightmost point
		// and wrapping around
		int j;
		for (j = 0; j < hole_num_values; j++) {
			int pos = rightmost_index + j + hole_num_values+4;
			if (pos >= rightmost_index - hole_index_offset + hole_num_values*2 + 4)
				pos -= hole_num_values;
			points[main_right_index+2+j] = points[pos];
		}
		// insert the new verts at the end of the main polygon.
		points[main_right_index+2+j++] = points[main_right_index+2];
		points[main_right_index+2+j++] = points[main_right_index+3];
		points[main_right_index+2+j++] = points[main_right_index];
		points[main_right_index+2+j++] = points[main_right_index+1];

		main_values += hole_num_values + 4;

		// shift everything up to take the place of the
		// hole and trim the end of the vector
		int num_to_move = 0;
		for (int j = rightmost+1; j < num_splits-1; j++) {
			if (!finished[j])
				num_to_move += hole_point_counts[j];
		}
		
		for (int j = 0; j < num_to_move; j++) {
			points[rightmost_index-hole_index_offset+hole_num_values+4+j] =
				points[rightmost_index-hole_index_offset+hole_num_values+4+hole_num_values+j];
		}
		num_values += 4;
		points.erase(points.begin()+num_values, points.end());
	}

	print_points(points, num_values);

	for (int i = 0; i < num_values; i += 2) {
		int x = points[i];
		int y = points[i+1];
#ifdef BIG
		x = x * SCALE;
		y = y * SCALE;
#endif
		Poly_Point *p = new Poly_Point;
		p->x = x;
		p->y = y;
		p->is_ear_tip = false;
		pt = new DL_List::Node;
		pt->data = (void *)p;
		DL_List::add(vert_list, pt);
	}

	pt = vert_list->head;
	for (;;) {
		Poly_Point *p = (Poly_Point *)pt->data;
		Poly_Point *p2 = (Poly_Point *)pt->next->data;
		al_draw_line(p->x, p->y, p2->x, p2->y, al_map_rgb(0, 255, 0), 1);
		pt = pt->next;
		if (pt == vert_list->head)
			break;
	}
	al_flip_display();
	al_rest(3);

	int num_ears = 0;
	scan_for_ears(vert_list, ear_list, num_ears);

	DL_List::Node *node = vert_list->head;
	Poly_Point *tri[3];
	DL_List::Node *nodes[3];

	/* the commented out condition is the real one I want to satisfy,
	 * but the loop won't end that way (yet.)
	 */
	while (ear_list->size > 0) {
		tri[0] = (Poly_Point *)node->data;
		tri[1] = (Poly_Point *)node->next->data;
		tri[2] = (Poly_Point *)node->next->next->data;
		nodes[0] = node;
		nodes[1] = node->next;
		nodes[2] = node->next->next;

		DL_List::Node *ear = find_ear(ear_list, tri[1]);
		if (ear) {
			Triangle *t = new Triangle;
			t->pts[0] = *tri[0];
			t->pts[1] = *tri[1];
			t->pts[2] = *tri[2];

			triangles.push_back(t);

			DL_List::remove(vert_list, nodes[1]);

			if (is_ear(vert_list, nodes[0])) {
				if (!tri[0]->is_ear_tip) {
					DL_List::Node *n = new DL_List::Node;
					n->data = tri[0];
					DL_List::add(ear_list, n);
					tri[0]->is_ear_tip = true;
				}
			}
			else {
				if (tri[0]->is_ear_tip) {
					DL_List::Node *ear2 = find_ear(ear_list, tri[0]);
					DL_List::remove(ear_list, ear2);
					tri[0]->is_ear_tip = false;
				}
			}
			if (is_ear(vert_list, nodes[2])) {
				if (!tri[2]->is_ear_tip) {
					DL_List::Node *n = new DL_List::Node;
					n->data = tri[2];
					DL_List::add(ear_list, n);
					tri[2]->is_ear_tip = true;
				}
			}
			else {
				if (tri[2]->is_ear_tip) {
					DL_List::Node *ear2 = find_ear(ear_list, tri[2]);
					DL_List::remove(ear_list, ear2);
					tri[2]->is_ear_tip = false;
				}
			}
	
			DL_List::remove(ear_list, ear);
		}
		else
			node = node->next;
	}

	delete[] splits;
}

int main(int argc, char **argv)
{
#ifdef HAVE_ALLEGRO
	al_init();
	al_init_primitives_addon();
	al_init_image_addon();
	ALLEGRO_DISPLAY *display = al_create_display(600, 600);
	(void)display;

	float start = al_current_time();
#endif

	std::list<Triangle *> triangles;
	get_triangles(start_points, splits, num_splits, triangles);

#ifdef HAVE_ALLEGRO
	float end = al_current_time();
	printf("%zu triangles in %g seconds\n", triangles.size(), end-start);
#endif

	std::list<Triangle *>::iterator it;
	for (it = triangles.begin(); it != triangles.end(); it++) {
		Triangle *t = *it;
		printf("%g, %g, %g, %g, %g, %g\n", t->pts[0].x, t->pts[0].y, t->pts[1].x, t->pts[1].y, t->pts[2].x, t->pts[2].y);
	}
	
#ifdef HAVE_ALLEGRO
#ifdef BIG
	ALLEGRO_BITMAP *bmp = al_create_bitmap(al_get_display_width(display)*SCALE, al_get_display_height(display)*SCALE);
	al_set_target_bitmap(bmp);
	al_clear_to_color(al_map_rgb(0, 0, 0));
#endif
	for (it = triangles.begin(); it != triangles.end(); it++) {
		Triangle *t = *it;
		//draw_tri(t, al_map_rgb(255, 0, 0));
		al_draw_triangle(t->pts[0].x, t->pts[0].y, t->pts[1].x, t->pts[1].y, t->pts[2].x, t->pts[2].y,
			al_map_rgb(255, 0, 0), 1);
	}
#ifdef BIG
	al_save_bitmap("big.png", bmp);
#else
	al_flip_display();
	al_rest(10);
#endif
#endif
	return 0;
}


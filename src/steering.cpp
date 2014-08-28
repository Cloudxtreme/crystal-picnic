#include <set>
#include <cmath>
#include "steering.h"

static int num_boids = 0;
static Steering::Boid *boids;

struct BoidCompare_x {
	bool operator()(const Steering::Boid *a, const Steering::Boid *b) const
	{
		return a->x < b->x;
	}
};
struct BoidCompare_y {
	bool operator()(const Steering::Boid *a, const Steering::Boid *b) const
	{
		return a->y < b->y;
	}
};
struct BoidCompare_z {
	bool operator()(const Steering::Boid *a, const Steering::Boid *b) const
	{
		return a->z < b->z;
	}
};
std::set<Steering::Boid *, BoidCompare_x> boid_x;
std::set<Steering::Boid *, BoidCompare_y> boid_y;
std::set<Steering::Boid *, BoidCompare_z> boid_z;
void unset_flags(void)
{
	for (int i = 0; i < num_boids; i++) {
		boids[i].flag = false;
	}
}
void set_flags(Steering::Boid *b)
{
	const int MAX_DIST = 30;

	std::set<Steering::Boid *, BoidCompare_x>::iterator it1;
	std::set<Steering::Boid *, BoidCompare_y>::iterator it2;
	std::set<Steering::Boid *, BoidCompare_z>::iterator it3;

	for (it1 = boid_x.begin(); it1 != boid_x.end(); it1++) {
		if (abs((*it1)->x-b->x) < MAX_DIST) {
			(*it1)->flag = true;
		}
		else if ((*it1)->x-MAX_DIST > b->x) {
			break;
		}
	}
	for (it2 = boid_y.begin(); it2 != boid_y.end(); it2++) {
		if ((*it2)->flag == false) continue;
		if (abs((*it2)->y-b->y) < MAX_DIST) {
			continue;
		}
		(*it2)->flag = false;
	}
	for (it3 = boid_z.begin(); it3 != boid_z.end(); it3++) {
		if ((*it3)->flag == false) continue;
		if (abs((*it3)->z-b->z) < MAX_DIST) {
			continue;
		}
		(*it3)->flag = false;
	}
}

namespace Steering {

const float BEE_MAX_A = 0.3;
const float BEE_MAX_V = 4;
const int BEE_SPREAD = 50; // boids will start -SPREAD -> SPREAD from the center
static General::Point<float> bee_target;
bool bee_chase;

float distance(Boid *b1, Boid *b2)
{
	float x = b1->x - b2->x;
	float y = b1->y - b2->y;
	float z = b1->z - b2->z;

	return sqrt(x*x + y*y + z*z);
}

void set_boids(int num, Boid *b)
{
	num_boids = num;
	boids = b;
}

void set_bee_chase(bool chase)
{
	bee_chase = chase;
}

void set_bee_target(General::Point<float> target)
{
	bee_target = target;
}

void fill_bee(Boid *b)
{
	float dx, dy;

	do {
		b->x = (bee_target.x) + (General::rand() % (BEE_SPREAD*2) - BEE_SPREAD);
		b->y = (bee_target.y) + (General::rand() % (BEE_SPREAD*2) - BEE_SPREAD);
		b->z = (General::rand() % (BEE_SPREAD*2) - BEE_SPREAD);
		b->vx = (General::rand() % RAND_MAX) / (float)RAND_MAX * BEE_MAX_V*2 - BEE_MAX_V;
		b->vy = (General::rand() % RAND_MAX) / (float)RAND_MAX * BEE_MAX_V*2 - BEE_MAX_V;
		b->vz = (General::rand() % RAND_MAX) / (float)RAND_MAX * BEE_MAX_V*2 - BEE_MAX_V;
		dx = (bee_target.x) - b->x;
		dy = (bee_target.y) - b->y;
	// this check makes the flock start in a circular pattern
	} while (sqrt(dx*dx + dy*dy + b->z*b->z) > BEE_SPREAD);
}

void sort_bees(Boid *boids)
{
	boid_x.clear();
	boid_y.clear();
	boid_z.clear();
	for (int i = 0; i < num_boids; i++) {
		boid_x.insert(&boids[i]);
		boid_y.insert(&boids[i]);
		boid_z.insert(&boids[i]);
	}
}

void update_bee(int idx, Boid *boids)
{
	const float MAX_A = 0.1f;
	const float MAX_V = 1.5f;
	float goto_x = 0, goto_y = 0, goto_z = 0, goto_n = 0;
	float away_x = 0, away_y = 0, away_z = 0, away_n = 0;
	float vx = 0, vy = 0, vz = 0;
	float dx, dy, dz;

	int mx = bee_target.x;
	int my = bee_target.y;

	if (boids[idx].x > mx+25 && bee_chase) {
		if (General::rand() % 3) {
			float a;
			a = General::rand() / (float)UINT_MAX
				* M_PI/2;
			if (General::rand() % 2) {
				a += M_PI * 3 / 2;
			}
			float x = cos(a) * MAX_V;
			float y = sin(a) * MAX_V;
			float nx = boids[idx].x + x;
			float ny = boids[idx].y + y;
			float nz = boids[idx].z;
			dx = mx - nx;
			dy = my - ny;
			dz = nz;
			if (sqrt(dx*dx + dy*dy + dz*dz) > 100) {
				a += M_PI;
				x = cos(a) * MAX_V;
				y = sin(a) * MAX_V;
			}
			boids[idx].x += x;
			boids[idx].y += y;
			return;
		}
	}

	dx = mx - boids[idx].x;
	dy = my - boids[idx].y;
	dz = boids[idx].z;
	if (sqrt(dx*dx + dy*dy + dz*dz) > 75) {
		goto_x = dx;
		goto_y = dy;
		goto_z = -dz;
		goto_n = 1;
	}
	else {
		// flag bees in range using sweep & prune technique
		unset_flags();
		set_flags(&boids[idx]);
		for (int i = 0; i < num_boids; i++) {
			if (i == idx) continue;
			if (boids[i].flag == false) continue;
			float dist = distance(&boids[i], &boids[idx]);
			if (dist < 5) {
				away_x += mx - boids[i].x;
				away_y += my - boids[i].y;
				away_z += (BEE_SPREAD*2) - boids[i].z;
				away_n++;
			}
			else {
				goto_x += mx - boids[i].x;
				goto_y += my - boids[i].y;
				goto_z += (BEE_SPREAD*2) - boids[i].z;
				goto_n++;
			}
		}
	}

	if (goto_n == 0 && away_n == 0)
		return;

	if (goto_n > 0) {
		vx = goto_x / goto_n;
		vy = goto_y / goto_n;
		vz = goto_z / goto_n;
	}

	if (away_n > 0) {
		vx -= away_x / away_n;
		vy -= away_y / away_n;
		vz -= away_z / away_n;
	}

	float total = fabs(vx) + fabs(vy) + fabs(vz);
	if (total == 0) return;
	vx /= total;
	vy /= total;
	vz /= total;

	boids[idx].vx += vx * MAX_A;
	boids[idx].vy += vy * MAX_A;
	boids[idx].vz += vz * MAX_A;
	if (boids[idx].vx > MAX_V) boids[idx].vx = MAX_V;
	if (boids[idx].vx < -MAX_V) boids[idx].vx = -MAX_V;
	if (boids[idx].vy > MAX_V) boids[idx].vy = MAX_V;
	if (boids[idx].vy < -MAX_V) boids[idx].vy = -MAX_V;
	if (boids[idx].vz > MAX_V) boids[idx].vz = MAX_V;
	if (boids[idx].vz < -MAX_V) boids[idx].vz = -MAX_V;

	boids[idx].x += boids[idx].vx;
	boids[idx].y += boids[idx].vy;
	boids[idx].z += boids[idx].vz;
}

} // end namespace Steering

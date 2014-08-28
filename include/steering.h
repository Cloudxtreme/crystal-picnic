#ifndef STEERING_H
#define STEERING_H

#include "general.h"

namespace Steering {

struct Boid
{
	float x, y, z;
	float vx, vy, vz;
	float flag;
};

float distance(Boid *b1, Boid *b2);
void set_boids(int num, Boid *b);

void set_bee_target(General::Point<float> target);
void fill_bee(Boid *b);
void sort_bees(Boid *boids);
void update_bee(int idx, Boid *boids);
void set_bee_chase(bool chase);

}

#endif

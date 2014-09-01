#ifndef ABILITIES_H
#define ABILITIES_H

namespace Abilities {

const int NUM_TIERS = 9;

struct Abilities {
	int abilities[3];
	int hp;
	int mp;
};

int count_crystals(Abilities &a);
void get_tier(Abilities &a, int *tier, int *tier_start, int *tier_end);

}

#endif

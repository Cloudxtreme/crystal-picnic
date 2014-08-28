#include "abilities.h"

namespace Abilities {

int count_crystals(Abilities &a)
{
	int total = 0;

	for (int i = 0; i < 3; i++) {
		total += a.abilities[i];
	}

	total += a.hp;
	total += a.mp;

	return total;
}

void get_tier(Abilities &a, int *tier, int *tier_start, int *tier_end)
{
	int tiers[NUM_TIERS][2] = {
		{ 0, 4 },
		{ 5, 9 },
		{ 10, 14 },
		{ 15, 16 },
		{ 17, 18 },
		{ 19, 20 },
		{ 21, 22 },
		{ 23, 24 },
		{ 25, 25 }
	};

	int t;
	int num = count_crystals(a);

	for (t = 0; t < NUM_TIERS; t++) {
		if (num <= tiers[t][1]) {
			break;
		}
	}

	if (tier) {
		*tier = t;
	}
	if (tier_start) {
		*tier_start = tiers[t][0];
	}
	if (tier_end) {
		*tier_end = tiers[t][1];
	}
}

}


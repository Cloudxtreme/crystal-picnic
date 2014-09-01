#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "well512.h"

/* initialize state to random bits */
static uint32_t state[16];
/* init should also reset this to 0 */
static unsigned int my_index = 0;

void WELLRNG512SEED()
{
	int i;

	srand(time(NULL));

	for (i = 0; i < 16; i++) {
		int r = rand();
		state[i] = (double)r/(uint32_t)RAND_MAX*(uint32_t)0xffffffff;
	}

	my_index = 0;
}

void WELLRNG512SEEDCONST(uint32_t a[16])
{
	int i;

	for (i = 0; i < 16; i++) {
		state[i] = a[i];
	}

	my_index = 0;
}

/* return 32 bit random number */
uint32_t WELLRNG512(void)
{
	uint32_t a, b, c, d;
	a = state[my_index];
	c = state[(my_index+13)&15];
	b = a^c^(a<<16)^(c<<15);
	c = state[(my_index+9)&15];
	c ^= (c>>11);
	a = state[my_index] = b^c;
	d = a^((a<<5)&(uint32_t)0xDA442D20);
	my_index = (my_index + 15)&15;
	a = state[my_index];
	state[my_index] = a^b^d^(a<<2)^(b<<18)^(c<<28);
	return state[my_index];
}


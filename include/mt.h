#ifndef MT_H
#define MT_H

#ifdef __cplusplus
extern "C" {
#endif

void init_genrand(unsigned long s);
unsigned long genrand_int32();
double genrand_real1();

//#include "SFMT.h"

#ifdef __cplusplus
}
#endif

#endif

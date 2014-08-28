#ifndef WELL512_H
#define WELL512_H

#ifdef __cplusplus
extern "C" {
#endif
void WELLRNG512SEED();
void WELLRNG512SEEDCONST(uint32_t a[16]);
uint32_t WELLRNG512();
#ifdef __cplusplus
}
#endif

#endif

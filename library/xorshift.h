#ifndef ICFPC2023_XORSHIFT_H
#define ICFPC2023_XORSHIFT_H

#include "stdint.h"

uint32_t xor32(void) {
  static uint32_t y = 2463534242;
  y = y ^ (y << 13); y = y ^ (y >> 17);
  return y = y ^ (y << 5);
}

uint32_t xor64(void) {
  static uint64_t x = 88172645463325252ULL;
  x = x ^ (x << 13); x = x ^ (x >> 7);
  return x = x ^ (x << 17);
}

uint32_t xor96(void) {
  static uint32_t x = 123456789;
  static uint32_t y = 362436069;
  static uint32_t z = 521288629;
  uint32_t t;

  t = (x ^ (x << 3)) ^ (y ^ (y >> 19)) ^ (z ^ (z << 6));
  x = y; y = z;
  return z = t;
}

uint32_t xor128(void) { 
  static uint32_t x = 123456789;
  static uint32_t y = 362436069;
  static uint32_t z = 521288629;
  static uint32_t w = 88675123; 
  uint32_t t;
 
  t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)); 
}

#endif //ICFPC2023_XORSHIFT_H
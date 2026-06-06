#include "int.h"
uint16_t get16(uint8_t *p) { return ((uint16_t)p[1] << 8) | (uint16_t)p[0]; }
uint32_t get32(uint8_t *p) {
  return ((uint32_t)get16(p + 2) << 16) | (uint32_t)get16(p);
}
void put16(uint8_t *p, uint16_t x) {
  p[0] = x;
  p[1] = x >> 8;
}
void put32(uint8_t *p, uint32_t x) {
  put16(p, x);
  put16(p + 2, x >> 16);
}

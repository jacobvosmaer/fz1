#include "fz.h"
#include "int.h"
/* isvoice guesses if p points to a non-empty voice */
int isvoice(uint8_t *p) {
  uint16_t loop = get16(p + 0x10);
  uint32_t wavst = get32(p), waved = get32(p + 4), genst = get32(p + 8),
           gened = get32(p + 12);
  return wavst <= genst && genst <= gened && gened <= waved &&
         (loop == 0x01d7 || loop == 0x101d || loop == 0x2014 || loop == 0x0013);
}
char *voicename(uint8_t *p) { return (char *)p + 0xb2; }
char *bankname(uint8_t *p) { return (char *)p + 0x282; }

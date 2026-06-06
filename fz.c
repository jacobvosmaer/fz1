#include "fz.h"
#include "int.h"
char *voicename(uint8_t *p) { return (char *)p + 0xb2; }
char *bankname(uint8_t *p) { return (char *)p + 0x282; }

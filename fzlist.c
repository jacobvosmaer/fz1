/* fzlist: print FZ-1 floppy image contents */
#include "fail.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define NSECTOR 1280
#define SECTORSIZE 1024
uint8_t disk[NSECTOR * SECTORSIZE + 1];
char *PROGNAME = "fzlist", *types = "FVBESP";
int u16(uint8_t *p) { return ((int)p[1] << 8) + (int)p[0]; }
int main(int argc, char **argv) {
  FILE *f;
  uint8_t *p;
  int n;
  if (argc != 2)
    fail("usage: %s IMAGE\n", PROGNAME);
  if (f = fopen(argv[1], "rb"), !f)
    fail("open %s: %s", argv[1], strerror(errno));
  if (n = fread(disk, 1, sizeof(disk), f), n != sizeof(disk) - 1)
    fail("invalid image size: %d", n);
  printf("File: %s\nLabel: %12.12s\n", argv[1], disk);
  for (p = disk + SECTORSIZE; *p && p < disk + 2 * SECTORSIZE; p += 16) {
    uint8_t *q = disk + SECTORSIZE * u16(p + 14), *qend = q + SECTORSIZE;
    int s0, s1;
    n = 0;
    for (s0 = u16(q), s1 = u16(q + 2); s0 && q < qend;
         q += 4, s0 = u16(q), s1 = u16(q + 2))
      n += s1 - s0 + 1;
    printf("%c  %9d  %12.12s\n", types[p[12]], n * SECTORSIZE, p);
  }
  return 0;
}

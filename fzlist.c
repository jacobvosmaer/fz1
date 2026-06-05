/* fzlist: print FZ-1 floppy image contents */
#include "fail.h"
#include "int.h"
#include <stdint.h>
#include <stdio.h>
#define NSECTOR 1280
#define SECTORSIZE 1024
uint8_t disk[NSECTOR * SECTORSIZE + 1], *diskend = disk + sizeof(disk) - 1;
char *PROGNAME = "fzlist", *types = "FVBESP";
int main(int argc, char **argv) {
  FILE *f;
  int n;
  if (argc != 2)
    fail("usage: %s IMAGE\n", PROGNAME);
  if (f = fopen(argv[1], "rb"), !f)
    failerrno("open %s", argv[1]);
  if (n = fread(disk, 1, sizeof(disk), f), n != sizeof(disk) - 1)
    fail("invalid image size: %d", n);
  printf("File: %s\nLabel: %12.12s\n", argv[1], disk);
  for (uint8_t *p = disk + SECTORSIZE; *p && p < disk + 2 * SECTORSIZE;
       p += 16) {
    /* An FZ-1 file is a series of ranges of sectors. This allows the FZ-1 to
     * re-use sectors after a file has been deleted. To find out the size of a
     * file we must sum the sizes of the ranges. Variable q points to the first
     * range in the extent table of the current file. */
    uint8_t *q = disk + SECTORSIZE * get16(p + 14), *qend = q + SECTORSIZE,
            filetype = p[12];
    if (!(q < diskend && qend <= diskend))
      fail("invalid file head address at offset %ld", p - disk);
    n = 0;
    for (; get16(q) && q < qend; q += 4)
      n += get16(q + 2) - get16(q) + 1;
    if (filetype > 5)
      fail("invalid filetype at offset %ld", p - disk);
    printf("%c  %9d  %12.12s\n", types[filetype], n * SECTORSIZE, p);
  }
  return 0;
}

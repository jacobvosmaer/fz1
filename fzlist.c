/* fzlist: print FZ-1 floppy image contents */
#include "fail.h"
#include "fz.h"
#include "int.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NSECTOR 1280
#define SECTORSIZE 1024
uint8_t disk[NSECTOR * SECTORSIZE + 1], *diskend = disk + sizeof(disk) - 1,
                                        buf[NSECTOR * SECTORSIZE];
char *PROGNAME = "fzlist", *types = "fvbesp";
void usage(void) {
  fprintf(stderr, "usage: %s [-l] IMAGE\n", PROGNAME);
  exit(1);
}
int main(int argc, char **argv) {
  FILE *f;
  int n, detail = 0;
  char *filename = argv[argc - 1];
  if (argc < 2 || argc > 3)
    usage();
  if (argc == 3) {
    if (strcmp(argv[1], "-l"))
      usage();
    else
      detail = 1;
  }
  if (f = fopen(filename, "rb"), !f)
    failerrno("open %s", filename);
  if (n = fread(disk, 1, sizeof(disk), f), n != sizeof(disk) - 1)
    fail("invalid image size: %d", n);
  printf("File: %s\nLabel: %12.12s\n", filename, disk);
  for (uint8_t *p = disk + SECTORSIZE; *p && p < disk + 2 * SECTORSIZE;
       p += 16) {
    /* An FZ-1 file is a series of ranges of sectors. This allows the FZ-1 to
     * re-use sectors after a file has been deleted. To find out the size of a
     * file we must sum the sizes of the ranges. Variable q points to the first
     * range in the extent table of the current file. */
    uint8_t *q = disk + SECTORSIZE * get16(p + 14), *qend = q + SECTORSIZE,
            filetype = p[12], *bufp = buf;
    if (!(q < diskend && qend <= diskend))
      fail("invalid file head address at offset %ld", p - disk);
    n = 0;
    for (; get16(q) && q < qend; q += 4) {
      int dbplen = get16(q + 2) - get16(q) + 1, datalen = dbplen * SECTORSIZE;
      n += dbplen;
      memmove(bufp, disk + get16(q) * SECTORSIZE, datalen);
      bufp += datalen;
    }
    if (filetype > 5)
      fail("invalid filetype at offset %ld", p - disk);
    printf("%c  %9d  %12.12s\n", types[filetype], n * SECTORSIZE, p);
    if (detail) {
      int nbank = get16(buf + 1018), nvoice = get16(buf + 1020);
      if (filetype == 0) {
        for (int i = 0; i < nbank; i++) {
          uint8_t *b = buf + SECTORSIZE * (1 + i);
          if (b < bufp)
            printf("                %c %12.12s\n", types[2], bankname(b));
        }
      }
      if (filetype == 2 || filetype == 0) {
        for (int i = 0; i < nvoice; i++) {
          uint8_t *v = buf + (1 + nbank) * SECTORSIZE + 256 * i;
          if (v < bufp)
            printf("                %c %12.12s\n", types[1], voicename(v));
        }
      }
    }
  }
  return 0;
}

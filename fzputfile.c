/* fzputfile: put a file onto a Casio FZ disk image */

#include "fail.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *PROGNAME = "fzputfile";

#define NSECTOR 1280
#define SECTORSIZE 1024
uint8_t disk[NSECTOR * SECTORSIZE], *CAT = disk + 128, *dir = disk + SECTORSIZE,
                                    *dirend = disk + 2 * SECTORSIZE;

int putint(int x, int width, uint8_t *p) {
  int i;
  assert(width > 0 && !(width % 8));
  for (i = 0; i < width; i += 8)
    *p++ = (x >> i);
  return width / 8;
}

uint8_t *sectoraddr(int sector) { return disk + sector * SECTORSIZE; }

int newsector(void) {
  int sector;
  for (sector = 2; CAT[sector / 8] & (1 << (sector % 8)) && sector < NSECTOR;
       sector++)
    ;
  if (sector == NSECTOR)
    fail("no space left on disk");
  CAT[sector / 8] |= 1 << (sector % 8);
  return sector;
}

int isname(uint8_t *p) {
  int i;
  for (i = 0; i < 12; i++)
    if (p[i] < 0x20 || p[i] > 0x7e)
      return 0;
  return 1;
}

int main(int argc, char **argv) {
  FILE *img, *file;
  uint8_t *direntry, *filehead, *dbp, buf[SECTORSIZE];
  int filetype, sector, n, nbank = 0, nvoice = 0, nwave = 0;
  char *filename = 0;
  if (argc != 4) {
    fprintf(stderr, "Usage: %s IMAGE TYPE FILE\n", PROGNAME);
    fputs("Supported file types: 0 (Full Dump Data), 1 (Voice Data)\n", stderr);
    return 1;
  }
  if (img = fopen(argv[1], "rb+"), !img)
    fail("open failed: %s", argv[1]);
  if (filetype = atoi(argv[2]), filetype < 0 || filetype > 1)
    fail("unsupported file type: %d", filetype);
  if (file = fopen(argv[3], "rb"), !file)
    fail("open failed: %s", argv[3]);
  if (!fread(disk, sizeof(disk), 1, img))
    fail("read %s: short read", argv[1]);
  for (direntry = dir; *direntry && direntry < dirend; direntry += 16)
    ;
  if (direntry == dirend)
    fail("directory table full");
  sector = newsector();
  putint(filetype, 16, direntry + 12);
  putint(sector, 16, direntry + 14);
  filehead = sectoraddr(sector);
  memset(filehead, 0, SECTORSIZE);
  dbp = filehead;
  putint(sector, 16, dbp);
  while (n = fread(buf, 1, sizeof(buf), file), n > 0) {
    uint8_t *p;
    int nextsector = newsector();
    if (nextsector != sector + 1) {
      if (dbp - filehead == 256)
        fail("too many data block pointers");
      dbp += 4;
      putint(nextsector, 16, dbp);
    }
    sector = nextsector;
    putint(sector, 16, dbp + 2);
    p = sectoraddr(sector);
    memmove(p, buf, n);
    switch (filetype) {
    case 0:
      filename = "FULL-DATA-FZ";
      /* Annoyingly, FZF files as found on the internet are missing their
       * bank/voice/wave layout bytes. We use heuristics to guess what they are.
       */
      if (!nvoice && !nwave && nbank < 8 && isname(p + 0x282)) {
        nbank++;
      } else if (!nwave && nvoice < 64 && !(nvoice % 4) && isname(p + 0xb2)) {
        int i;
        for (i = 0; i < 4 && isname(p + i * 256 + 0xb2); i++)
          nvoice++;
      } else {
        nwave++;
      }
      break;
    case 1:
      nbank = 0;
      nvoice = 1;
      if (!filename) /* First sector of voice file */
        filename = (char *)p + 178;
      else
        nwave++;
      break;
    }
  }
  if (ferror(file))
    fail("file read error");
  assert(filename);
  memmove(direntry, filename, 12);
  assert(nbank >= 0 && nbank <= 8);
  putint(nbank, 16, filehead + 1018);
  assert(nvoice >= 0 && nvoice <= 64);
  putint(nvoice, 16, filehead + 1020);
  assert(nwave >= 0 && nwave < NSECTOR);
  putint(nwave, 16, filehead + 1022);
  if (fseek(img, 0, SEEK_SET))
    fail("fseek image failed");
  if (!fwrite(disk, sizeof(disk), 1, img))
    fail("fwrite failed");
  if (fclose(img))
    fail("close image failed");
  return 0;
}

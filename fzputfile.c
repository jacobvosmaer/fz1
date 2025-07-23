
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

int getuint(uint8_t *p, int width) {
  int x = 0, i;
  assert(width > 0 && !(width % 8));
  for (i = 0; i < width; i += 8)
    x |= (int)*p++ << i;
  return x;
}

int getint(uint8_t *p, int width) {
  int x = getuint(p, width);
  if (x >= (1 << (width - 1)))
    x -= 1 << width;
  return x;
}

uint8_t *sectoraddr(int sector) { return disk + sector * SECTORSIZE; }

int newsector(void) {
  int sector;
  for (sector = 2; CAT[sector / 8] & (1 << (sector % 8)) && sector < NSECTOR;
       sector++)
    ;
  if (sector == NSECTOR)
    fail("no space left in CAT");
  CAT[sector / 8] |= 1 << (sector % 8);
  return sector;
}

int main(int argc, char **argv) {
  FILE *img, *file;
  uint8_t *direntry, *filehead, *dbp, buf[SECTORSIZE];
  int filetype, sector, n, filefirst, filesectors;
  if (argc != 4) {
    fprintf(stderr, "Usage: %s IMAGE TYPE FILE\n", PROGNAME);
    return 1;
  }
  if (img = fopen(argv[1], "rb+"), !img)
    fail("open failed: %s", argv[1]);
  if (filetype = atoi(argv[2]), filetype != 1)
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
  filefirst = 0;
  filesectors = 0;
  while (n = fread(buf, 1, sizeof(buf), file), n > 0) {
    int nextsector = newsector();
    if (nextsector != sector + 1) {
      if (dbp - filehead == 256)
        fail("too many data block pointers");
      dbp += 4;
      putint(nextsector, 16, dbp);
    }
    sector = nextsector;
    putint(sector, 16, dbp + 2);
    memmove(sectoraddr(sector), buf, n);
    if (!filefirst)
      filefirst = sector;
    filesectors++;
  }
  if (ferror(file))
    fail("file read error");
  switch (filetype) {
  case 1:
    memmove(direntry, sectoraddr(filefirst) + 178, 12);
    putint(0, 16, filehead + 1018); /* 0 banks */
    putint(1, 16, filehead + 1020); /* 1 voice */
    putint(filesectors - 1, 16,
           filehead + 1022); /* filesectors-1 PCM data sectors */
    break;
  default:
    fail("unknown filetype: %d", filetype);
    break;
  }
  if (fseek(img, 0, SEEK_SET))
    fail("fseek image failed");
  if (!fwrite(disk, sizeof(disk), 1, img))
    fail("fwrite failed");
  return 0;
}

/* fzputfile: put a file onto a Casio FZ disk image */
#include "fail.h"
#include "int.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
char *PROGNAME = "fzputfile";
#define NSECTOR 1280
#define SECTORSIZE 1024
enum { FULL = 0, VOICE = 1, BANK = 2 };
uint8_t disk[NSECTOR * SECTORSIZE], *CAT = disk + 128, *dir = disk + SECTORSIZE,
                                    *dirend = disk + 2 * SECTORSIZE;
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
  int nonzero = 0, nonprint = 0;
  for (int i = 0; i < 12; i++) {
    nonprint += p[i] < 0x20 || p[i] > 0x7e;
    nonzero += p[i] > 0;
  }
  return !nonzero || !nonprint;
}
int isvoice(uint8_t *p) {
  uint16_t magic = get16(p + 0x10);
  return isname(p + 0xb2) && (magic == 0x01d7 || magic == 0x101d ||
                              magic == 0x2014 || magic == 0x0013);
}
int main(int argc, char **argv) {
  FILE *img, *file;
  uint8_t *direntry, *filehead, *dbp, buf[SECTORSIZE] = {0};
  int filetype, sector, nbank = 0, nvoice = 0, nwave = 0;
  char *filename = 0;
  if (argc != 4) {
    fprintf(stderr, "Usage: %s IMAGE TYPE FILE\n", PROGNAME);
    fputs("Supported file types: 0 (Full Dump Data), 1 (Voice Data), 2 (Bank "
          "Data)\n",
          stderr);
    return 1;
  }
  if (img = fopen(argv[1], "rb+"), !img)
    fail("open failed: %s", argv[1]);
  if (strlen(argv[2]) != 1 || argv[2][0] < '0' || argv[2][0] > '2')
    fail("unsupported file type: %s", argv[2]);
  filetype = argv[2][0] - '0';
  if (file = fopen(argv[3], "rb"), !file)
    fail("open failed: %s", argv[3]);
  if (!fread(disk, sizeof(disk), 1, img))
    fail("read %s: short read", argv[1]);
  for (direntry = dir; *direntry && direntry < dirend; direntry += 16)
    ;
  if (direntry == dirend)
    fail("directory table full");
  sector = newsector();
  put16(direntry + 12, filetype);
  put16(direntry + 14, sector);
  filehead = sectoraddr(sector);
  memset(filehead, 0, SECTORSIZE);
  dbp = filehead;
  put16(dbp, sector);
  put16(dbp + 2, sector);
  while (fread(buf, 1, sizeof(buf), file)) {
    uint8_t *p;
    int nextsector = newsector();
    if (nextsector != sector + 1) {
      if (dbp - filehead == 256)
        fail("too many data block pointers");
      dbp += 4;
      put16(dbp, nextsector);
    }
    sector = nextsector;
    put16(dbp + 2, sector);
    p = sectoraddr(sector);
    memmove(p, buf, sizeof(buf));
    switch (filetype) {
    case BANK: /* Use FULL heuristics */
    case FULL:
      /* Annoyingly, FZF files as found on the internet are missing their
       * bank/voice/wave layout bytes. We use heuristics to guess what they are.
       */
      if (!filename)
        filename = filetype == FULL ? "FULL-DATA-FZ" : (char *)p + 0x282;
      if (!nvoice && !nwave && nbank < (filetype == FULL ? 8 : 1) &&
          isname(p + 0x282) && !isvoice(p))
        nbank++;
      else if (!nwave && nvoice < 64 && !(nvoice % 4) && isvoice(p))
        for (int i = 0; i < 4 && isvoice(p + i * 256); i++)
          nvoice++;
      else
        nwave++;
      break;
    case VOICE:
      if (!filename) { /* First sector of voice file */
        filename = (char *)p + 0xb2;
        nbank = 0;
        nvoice = 1;
      } else {
        nwave++;
      }
      break;
    }
  }
  if (ferror(file))
    fail("file read error");
  if (filetype == BANK && nbank != 1)
    fail("expected 1 bank in FZB file, got %d", nbank);
  if (!filename)
    fail("error reading input file");
  memmove(direntry, filename, 12);
  assert(nbank >= 0 && nbank <= 8);
  put16(filehead + 1018, nbank);
  assert(nvoice >= 0 && nvoice <= 64);
  put16(filehead + 1020, nvoice);
  assert(nwave >= 0 && nwave < NSECTOR);
  put16(filehead + 1022, nwave);
  if (fseek(img, 0, SEEK_SET))
    fail("fseek image failed");
  if (!fwrite(disk, sizeof(disk), 1, img))
    fail("fwrite failed");
  if (fclose(img))
    fail("close image failed");
  return 0;
}

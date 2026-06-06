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
int isvoice(uint8_t *p) {
  uint16_t magic = get16(p + 0x10);
  uint32_t wavst = get32(p), waved = get32(p + 4), genst = get32(p + 8),
           gened = get32(p + 12);
  return wavst <= genst && genst <= gened && gened <= waved &&
         (magic == 0x01d7 || magic == 0x101d || magic == 0x2014 ||
          magic == 0x0013);
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
    case FULL:
      filename = "FULL-DATA-FZ";
      /* FZF files as found on the internet are missing their bank/voice/wave
       * layout bytes. We use heuristics to guess what they are.
       *
       * We expect 0-8 banks, followed by 0-64 voices, and everything else that
       * follows is wave data.
       *
       * The first byte in a bank is the number of areas in the bank. The SAVE
       * FULL function omits banks with 0 areas so if the first byte of the
       * current sector is 0, it cannot be a bank. Conversely, the first voice
       * will have its sample data start at sample address 0 so the moment we
       * are out of banks we will see a 0 at *p.
       */
      if (!nvoice && !nwave && nbank < 8 && *p) { /* bank sector */
        nbank++;
      } else if (!nwave && !(nvoice % 4) && nvoice < 64 &&
                 isvoice(p)) { /* voice sector */
        for (uint8_t *q = p; q < p + SECTORSIZE && isvoice(q); q += 256)
          nvoice++;
      } else { /* wave sector */
        nwave++;
      }
      break;
    case VOICE:
      if (!filename) { /* voice sector */
        filename = (char *)p + 0xb2;
        nbank = 0;
        nvoice = 1;
      } else { /* wave sector */
        nwave++;
      }
      break;
    case BANK:
      if (!filename) { /* bank sector */
        filename = (char *)p + 0x282;
        nbank = 1;
      } else if (!nwave && !(nvoice % 4) && nvoice < 64 && isvoice(p)) {
        for (uint8_t *q = p; q < p + SECTORSIZE && isvoice(q); q += 256)
          nvoice++;
      } else { /* wave sector */
        nwave++;
      }
      break;
    }
  }
  if (ferror(file))
    fail("file read error");
  if (!filename)
    fail("error reading input file");
  if (!*filename)
    filename = "???         ";
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

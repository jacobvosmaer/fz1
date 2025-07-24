/* fzbuildfull: build a Casio FZ-1 "Full Data Dump" from invidual bank and voice
 * files. */
#include "fail.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
uint8_t fzf[2 * 1277 * 1024],
    effectdefault[] = {0x18, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
int fzfsize;
char *PROGNAME = "fzbuildfull";
void fixoffsets(uint8_t *buf, int offset) {}
int main(int argc, char **argv) {
  int i;
  FILE *fout;
  uint8_t *voicep = fzf, *wavestart = fzf + 1024 * ((argc - 2 + 3) / 4),
          *wavep = wavestart, buf[1024] = {0};
  if (argc > 66)
    fail("maximum number of voices is 64, got %d", argc - 2);
  for (i = 2; i < argc; i++) {
    FILE *f = fopen(argv[i], "rb");
    if (!f)
      fail("cannot open %s", argv[i]);
    if (!fread(buf, sizeof(buf), 1, f))
      fail("short read on first block of voice in %s", argv[i]);
    memmove(voicep, buf, 192);
    if (i == 2)
      memmove(voicep + 960, effectdefault, sizeof(effectdefault));
    fixoffsets(voicep, wavep - wavestart);
    voicep += 256;
    while (fread(buf, sizeof(buf), 1, f)) {
      if (wavep - fzf == sizeof(fzf))
        fail("output file too large");
      memmove(wavep, buf, sizeof(buf));
      wavep += 1024;
    }
    fclose(f);
  }
  if (fout = fopen(argv[1], "wb"), !fout)
    fail("cannot open %s", argv[1]);
  if (!fwrite(fzf, wavep - fzf, 1, fout))
    fail("write fzf failed");
  if (fclose(fout))
    fail("close %s failed", argv[1]);
  return 0;
}

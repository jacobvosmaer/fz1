/* fzbuildfull: build a Casio FZ-1 "Full Data Dump" from invidual voice files.
 */
#include "fail.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
uint8_t fzf[2 * 1277 * 1024];
/* default global settings for pitch bend range, MIDI channel etc. */
uint8_t effectdefault[] = {0x18, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
char *PROGNAME = "fzbuildfull";
uint32_t getint(uint8_t *p, int width) {
  int i;
  uint32_t x = 0;
  assert(width > 0 && width <= 32 && !(width % 8));
  for (i = 0; i < width; i += 8)
    x += *p++ << i;
  return x;
}
void putint(uint32_t x, uint8_t *p, int width) {
  assert(width > 0 && width <= 32 && !(width % 8));
  for (; width; width -= 8) {
    *p++ = x;
    x >>= 8;
  }
}
void fixsampleoffsets(uint8_t *voice, int offset) {
  uint8_t *p;
  offset /= 2; /* convert byte offset to sample offset */
  /* See CASIO DIGITAL SAMPLING KEYBOARD MODEL FZ-1 DATA STRUCTURES document for
   * voice struct layout. We are fixing fields wavest, waved, genst, gened,
   * loopst[MAXE] and looped[MAXE]. */
  for (p = voice; p < voice + 0x10; p += 4)
    putint(getint(p, 32) + offset, p, 32);
  for (p = voice + 0x14; p < voice + 0x54; p += 4)
    putint(getint(p, 32) + offset, p, 32);
}
int main(int argc, char **argv) {
  int i, nvoice = argc - 2, voicesectors = (nvoice + 3) / 4;
  FILE *fout;
  uint8_t *voicep = fzf, *wavestart = fzf + 1024 * voicesectors,
          *wavep = wavestart;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s FZF_FILE VOICE [VOICE...]\n", PROGNAME);
    return 1;
  }
  if (nvoice > 64)
    fail("maximum number of voices is 64, got %d", nvoice);
  memmove(fzf + 960, effectdefault, sizeof(effectdefault));
  for (i = 2; i < argc; i++) {
    uint8_t buf[1024];
    FILE *f = fopen(argv[i], "rb");
    if (!f)
      fail("cannot open %s", argv[i]);
    if (!fread(buf, sizeof(buf), 1, f))
      fail("short read on first block of voice in %s", argv[i]);
    memmove(voicep, buf, 192);
    fixsampleoffsets(voicep, wavep - wavestart);
    voicep += 256;
    while (fread(buf, sizeof(buf), 1, f)) {
      if (wavep - fzf == sizeof(fzf))
        fail("output file too large");
      memmove(wavep, buf, sizeof(buf));
      wavep += sizeof(buf);
    }
    if (ferror(f))
      fail("error reading from %s", argv[1]);
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

/* fzbuildfull: build a Casio FZ-1 "Full Data Dump" from invidual voice files.
 */
#include "fail.h"
#include "int.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
uint8_t bank[2 * 1277 * 1024];
char *PROGNAME = "fzbuildbank";
void fixsampleoffsets(uint8_t *voice, int offset) {
  offset /= 2; /* convert byte offset to sample offset */
  /* See CASIO DIGITAL SAMPLING KEYBOARD MODEL FZ-1 DATA STRUCTURES document for
   * voice struct layout. We are fixing fields wavest, waved, genst, gened,
   * loopst[MAXE] and looped[MAXE]. */
  for (uint8_t *p = voice; p < voice + 0x10; p += 4)
    put32(p, get32(p) + offset);
  for (uint8_t *p = voice + 0x14; p < voice + 0x54; p += 4)
    put32(p, get32(p) + offset);
}
int main(int argc, char **argv) {
  int nvoice = argc - 2, voicesectors = (nvoice + 3) / 4;
  FILE *fout;
  uint8_t *voicep = bank + 1024, *wavestart = voicep + 1024 * voicesectors,
          *wavep = wavestart;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s FZB_FILE VOICE [VOICE...]\n", PROGNAME);
    return 1;
  }
  if (nvoice > 64)
    fail("maximum number of voices is 64, got %d", nvoice);
  put16(bank, nvoice);
  for (int i = 0; i < nvoice; i++) {
    /* 0x24 is the MIDI note number of the lowest key on the FZ-1 keyboard */
    bank[0x2 + i] = 0x24 + i;       /* key high */
    bank[0x42 + i] = 0x24 + i;      /* key low */
    bank[0x82 + i] = 0x7f;          /* velocity high */
    bank[0xc2 + i] = 1;             /* velocity low */
    bank[0x102 + i] = 0x24 + i;     /* key center */
    bank[0x104 + i] = 0;            /* MIDI channel */
    bank[0x182 + i] = 0xff;         /* audio outputs */
    put16(bank + 0x202 + 2 * i, i); /* voice number */
  }
  sprintf((char *)bank + 0x282, "%-12.12s", argv[1]);
  for (int i = 2; i < argc; i++) {
    uint8_t buf[1024];
    FILE *f = fopen(argv[i], "rb");
    if (!f)
      fail("cannot open %s", argv[i]);
    if (!fread(buf, sizeof(buf), 1, f))
      fail("short read on first block of voice in %s", argv[i]);
    fixsampleoffsets(buf, wavep - wavestart);
    memmove(voicep, buf, 192);
    voicep += 256;
    while (fread(buf, sizeof(buf), 1, f)) {
      if (wavep - bank == sizeof(bank))
        fail("output file too large");
      memmove(wavep, buf, sizeof(buf));
      wavep += sizeof(buf);
    }
    if (ferror(f))
      fail("error reading from %s", argv[i]);
    fclose(f);
  }
  if (fout = fopen(argv[1], "wb"), !fout)
    fail("cannot open %s", argv[1]);
  if (!fwrite(bank, wavep - bank, 1, fout))
    fail("write fzb failed");
  if (fclose(fout))
    fail("close %s failed", argv[1]);
  return 0;
}

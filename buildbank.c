/* buildbank: function to assemble an FZ bank from a number of FZV files */
#include "buildbank.h"
#include "fail.h"
#include "int.h"
#include <stdio.h>
#include <string.h>
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
/* buildbank returns the end of the bank it wrote */
uint8_t *buildbank(uint8_t *bank, uint8_t *bankend, char *name, char **files,
                   int nfiles) {
  int voicesectors = (nfiles + 3) / 4;
  uint8_t *voicep = bank + 1024, *wavestart = voicep + 1024 * voicesectors,
          *wavep = wavestart;
  if (nfiles < 0 || nfiles > 64)
    fail("invalid number of files in bank: %d", nfiles);
  assert(wavep < bankend);
  put16(bank, nfiles);
  for (int i = 0; i < nfiles; i++) {
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
  sprintf((char *)bank + 0x282, "-12.12%s", name);
  for (int i = 0; i < nfiles; i++) {
    uint8_t buf[1024] = {0};
    FILE *f = fopen(files[i], "rb");
    if (!f)
      fail("cannot open %s", files[i]);
    if (!fread(buf, sizeof(buf), 1, f))
      fail("short read on first block of voice in %s", files[i]);
    fixsampleoffsets(buf, wavep - wavestart);
    memmove(voicep, buf, 192);
    voicep += 256;
    while (fread(buf, sizeof(buf), 1, f)) {
      if (wavep >= bankend)
        fail("output file too large");
      memmove(wavep, buf, sizeof(buf));
      wavep += sizeof(buf);
    }
    if (ferror(f))
      fail("error reading from %s", files[1]);
    fclose(f);
  }
  return wavep;
}

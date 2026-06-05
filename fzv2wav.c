/* fzv2wav: extract audio from Casio FZ-1 .fzv voice file */
#include "fail.h"
#include "int.h"
#include "wav.h"
#include <stdint.h>
#include <stdio.h>
uint8_t fzv[2 * 1024 * 1024];
char *PROGNAME = "fzv2wav";
int main(int argc, char **argv) {
  int fzvsize, waveend, samplerate;
  FILE *f;
  if (argc != 3) {
    fprintf(stderr, "Usage: %s FZV WAV\n", PROGNAME);
    return 1;
  }
  if (f = fopen(argv[1], "rb"), !f)
    failerrno("open %s", argv[1]);
  if (fzvsize = fread(fzv, 1, sizeof(fzv), f),
      fzvsize == sizeof(fzv) || fzvsize < 1024)
    fail("invalid FZV file size: %d", fzvsize);
  fclose(f);
  if (get32(fzv))
    fail("invalid wave start");
  if (waveend = get32(fzv + 4), waveend > (fzvsize - 1024) / 2)
    fail("invalid wave end: %d", waveend);
  if (samplerate = fzv[0xb1], samplerate > 2)
    fail("invalid samplerate: %d", samplerate);
  if (f = fopen(argv[2], "wb"), !f)
    failerrno("open %s", argv[2]);
  if (!writewav(fzv + 1024, waveend, 36000 / (1 << samplerate), f))
    failerrno("write %s", argv[2]);
  if (fclose(f))
    failerrno("close %s", argv[2]);
  return 0;
}

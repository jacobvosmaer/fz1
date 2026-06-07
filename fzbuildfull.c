/* fzbuildfull: build a Casio FZ-1 "Full Data Dump" from invidual voice files.
 */
#include "buildbank.h"
#include "fail.h"
#include "int.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
uint8_t fzf[2 * 1277 * 1024];
/* default global settings for pitch bend range, MIDI channel etc. */
uint8_t effectdefault[] = {0x18, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
char *PROGNAME = "fzbuildfull";
int main(int argc, char **argv) {
  uint8_t *fzfend;
  FILE *fout;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s FZF_FILE VOICE [VOICE...]\n", PROGNAME);
    return 1;
  }
  fzfend = buildbank(fzf, fzf + sizeof(fzf), "All Voices", argv + 2, argc - 2);
  memmove(fzf + 960, effectdefault, sizeof(effectdefault));
  if (fout = fopen(argv[1], "wb"), !fout)
    fail("cannot open %s", argv[1]);
  if (!fwrite(fzf, fzfend - fzf, 1, fout))
    fail("write fzf failed");
  if (fclose(fout))
    fail("close %s failed", argv[1]);
  return 0;
}

/* fzbuildbank: build a Casio FZ-1 bank from invidual voice files. */
#include "buildbank.h"
#include "fail.h"
#include "int.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
uint8_t bank[2 * 1277 * 1024];
char *PROGNAME = "fzbuildbank";
int main(int argc, char **argv) {
  FILE *fout;
  uint8_t *bankend;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s FZB_FILE VOICE [VOICE...]\n", PROGNAME);
    return 1;
  }
  bankend = buildbank(bank, bank + sizeof(bank), argv[1], argv + 2, argc - 2);
  if (fout = fopen(argv[1], "wb"), !fout)
    fail("cannot open %s", argv[1]);
  if (!fwrite(bank, bankend - bank, 1, fout))
    fail("write fzb failed");
  if (fclose(fout))
    fail("close %s failed", argv[1]);
  return 0;
}

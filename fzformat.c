/* fzformat: create empty Casio FZ-1 disk image */
#include "fail.h"
#include <stdio.h>
#include <string.h>
char *PROGNAME = "fzformat";
unsigned char sector[1024];
int main(int argc, char **argv) {
  int i;
  FILE *f;
  if (argc != 3) {
    fprintf(stderr, "Usage: %s LABEL IMAGE\n", PROGNAME);
    return 1;
  }
  if (f = fopen(argv[2], "wb"), !f)
    fail("failed to opeb %s", argv[2]);
  memset(sector, 0, sizeof(sector));
  /* Trailing zero of snprintf is OK */
  snprintf((char *)sector, 13, "%-12.12s", argv[1]);
  sector[14] = 2;
  snprintf((char *)sector + 16, 13, "%-12.12s", argv[1]);
  sector[128] = 3; /* First two clusters are allocated */
  /* The Cluster Allocation Table has 6144 entries but the physical disk has
   * only 1280 clusters. So the last 4864 clusters are marked as allocated. */
  memset(sector + 0x120, 0xff, sizeof(sector) - 0x120);
  if (!fwrite(sector, sizeof(sector), 1, f))
    fail("fwrite sector 0");
  memset(sector, 0, sizeof(sector));
  if (!fwrite(sector, sizeof(sector), 1, f))
    fail("fwrite sector 1");
  memset(sector, 'Z', sizeof(sector));
  for (i = 2; i < 1280; i++)
    if (!fwrite(sector, sizeof(sector), 1, f))
      fail("fwrite sector %d", i);
  if (fclose(f))
    fail("failed to close %s", argv[1]);
  return 0;
}

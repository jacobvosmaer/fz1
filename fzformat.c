/* fzformat: create empty Casio FZ-1 disk image */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __clang__ || __GNUC__
void fail(char *fmt, ...) __attribute__((format(printf, 1, 2)));
#endif

void fail(char *fmt, ...) {
  va_list ap;
  fputs("fzformat: error: ", stderr);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(1);
}

unsigned char sector[1024];
#define NAMESIZE 12

int main(int argc, char **argv) {
  int i, n;
  if (argc != 2) {
    fputs("Usage: fzformat LABEL\n", stderr);
    return 1;
  }

  n = strlen(argv[1]);
  if (n > NAMESIZE)
    fail("label name too long (max NAMESIZE): %d", n);

  memset(sector, 0, sizeof(sector));
  memset(sector, ' ', NAMESIZE);
  memmove(sector, argv[1], n);
  sector[14] = 2;
  memset(sector + 16, ' ', NAMESIZE);
  memmove(sector + 16, argv[1], n);
  sector[128] = 3; /* First two clusters are allocated */
  memset(sector + 0x120, 0xff,
         sizeof(sector) -
             0x120); /* Cluster Allocation Table has 6144 entries but the
                        physical disk has only 1280 clusters. So the last 4864
                        clusters are marked as allocated. */
  if (!fwrite(sector, sizeof(sector), 1, stdout))
    fail("fwrite sector 0");

  memset(sector, 0, sizeof(sector));
  if (!fwrite(sector, sizeof(sector), 1, stdout))
    fail("fwrite sector 1");

  memset(sector, 'Z', sizeof(sector));
  for (i = 2; i < 1280; i++)
    if (!fwrite(sector, sizeof(sector), 1, stdout))
      fail("fwrite sector %d", i);
}

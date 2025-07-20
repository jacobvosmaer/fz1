
#include "fail.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void fail(char *fmt, ...) {
  va_list ap;
  fprintf(stderr, "%s: error: ", PROGNAME);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(1);
}

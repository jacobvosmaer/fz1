
#include "fail.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fail(char *fmt, ...) {
  va_list ap;
  fprintf(stderr, "%s: error: ", PROGNAME);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(1);
}

void failerrno(char *fmt, ...) {
  va_list ap;
  fprintf(stderr, "%s: error: ", PROGNAME);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, ": %s\n", strerror(errno));
  exit(1);
}

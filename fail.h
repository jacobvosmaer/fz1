#ifndef FAIL_H
#define FAIL_H

#if __clang__ || __GNUC__
void fail(char *fmt, ...) __attribute__((format(printf, 1, 2)));
#else
void fail(char *fmt, ...);
#endif

#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()

extern char *PROGNAME;

#endif

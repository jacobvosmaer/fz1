CFLAGS += -Wall -pedantic -std=gnu89
EXE = fzformat fzputfile fzbuildfull fzlist
OBJS = fail.o
PREFIX ?= /usr/local
all: $(EXE)
dev: CFLAGS += -Werror -g
dev: all
fzformat: fail.o
fzputfile: fail.o
fzbuildfull: fail.o
fzlist: fail.o
fail.o: fail.h
install: all
	install $(EXE) $(PREFIX)/bin/
clean:
	rm -f -- $(EXE) $(OBJS)

CFLAGS += -Wall -pedantic -std=gnu11 -fno-common
EXE = fzformat fzputfile fzbuildfull fzlist fzv2wav
OBJS = fail.o wav.o
PREFIX ?= /usr/local
all: $(EXE)
dev: CFLAGS += -Werror -g
dev: all
fzformat: fail.o
fzputfile: fail.o int.o
fzbuildfull: fail.o int.o
fzlist: fail.o int.o
fzv2wav: wav.o fail.o
fail.o: fail.h
wav.o: wav.h
install: all
	install wav2fzv $(EXE) $(PREFIX)/bin/
clean:
	rm -f -- $(EXE) $(OBJS)

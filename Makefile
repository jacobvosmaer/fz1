CFLAGS += -Wall -pedantic -std=gnu89
EXE = fzformat fzputfile fzbuildfull fzlist
OBJS = fail.o
all: $(EXE)
dev: CFLAGS += -Werror -g
dev: all
fzformat: fail.o
fzputfile: fail.o
fzbuildfull: fail.o
fzlist: fail.o
fail.o: fail.h
clean:
	rm -f -- $(EXE) $(OBJS)

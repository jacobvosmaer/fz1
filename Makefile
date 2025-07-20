CFLAGS += -Wall -pedantic -std=gnu89
EXE = fzformat fzputfile
OBJS = fail.o
all: $(EXE)
fzformat: fail.o
fzputfile: fail.o
fail.o: fail.h
clean:
	rm -f -- $(EXE) $(OBJS)

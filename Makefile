CFLAGS += -Wall -pedantic -std=gnu89
EXE = fzformat
OBJS = fail.o
all: $(EXE)
fzformat: fail.o
fail.o: fail.h
clean:
	rm -f -- $(EXE) $(OBJS)

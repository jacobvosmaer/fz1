CFLAGS += -Wall -pedantic -std=gnu89
EXE = fzformat
all: $(EXE)
clean:
	rm -f -- $(EXE)
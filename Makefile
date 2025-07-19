CFLAGS += -Wall -pedantic -std=gnu89
EXE = format
all: $(EXE)
clean:
	rm -f -- $(EXE)
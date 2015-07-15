CC=gcc
STRIP=strip
CFLAGS=-std=c11 -Wall -Werror -ggdb -O3 -g
INCLUDES      = -I.
SOURCES       = bitreverse.c ntt.c baileys.c swap.c mod.c twiddle.c
OBJECTS      = $(SOURCES:.c=.o)

.PRECIOUS: %.c %.h

.PHONY: all
all: test forward

test: $(OBJECTS) test.o

forward: $(OBJECTS) forward.o

.PHONY: tests run_tests
.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
	rm -f test $(OBJECTS) test.o forward.o


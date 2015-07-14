CC=gcc
STRIP=strip
CFLAGS=-std=c11 -Wall -Werror -ggdb -O0 -fsanitize=address
LDFLAGS=-fsanitize=address
INCLUDES      = -I.
SOURCES       = test.c bitreverse.c ntt.c baileys.c
OBJECTS      = $(SOURCES:.c=.o)

.PRECIOUS: %.c %.h

.PHONY: all
all: test

test: $(OBJECTS)

.PHONY: tests run_tests
.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
	rm -f test $(OBJECTS)


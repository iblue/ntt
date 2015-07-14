CC=gcc
STRIP=strip
CFLAGS=-std=c11 -Wall -Werror
INCLUDES      = -I.
SOURCES       = test.c bitreverse.c
OBJECTS      = $(SOURCES:.c=.o)

.PRECIOUS: %.c %.h

.PHONY: all
all: CFLAGS += -ggdb -O2
all: test

test: $(OBJECTS)

.PHONY: tests run_tests
.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
	rm -f test $(OBJECTS)


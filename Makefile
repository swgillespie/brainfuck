CC := clang

INCLUDEDIR := .
CFLAGS := -std=c99 -Wall -Werror -Wextra -pedantic -Wshadow -Wpointer-arith \
          -Wstrict-prototypes -Wmissing-prototypes -Wno-gnu-label-as-value  \
					-I$(INCLUDEDIR) -O2
NASMFLAGS := -g
LDFLAGS :=
PROGNAME := brainfuck


SOURCES := $(shell find . -name "*.c")
OBJECTS := $(SOURCES:.c=.o)

all: $(PROGNAME)

$(PROGNAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(PROGNAME)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(PROGNAME)
	rm -f $(OBJECTS)
	rm -f $(shell find . -name *~)

valgrind: all
	valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes --dsymutil=yes ./$(PROGNAME)

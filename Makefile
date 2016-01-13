CC := gcc

INCLUDEDIR := .
LUAJIT_DIR := luajit-2.0
MINILUA := minilua

JIT_DASM := src/jit.dasm
JIT_C := src/jit.temp.c
JIT_O := src/jit.o

FORMAT_STR := "[+] %4s %-20s -> %s\n"

CFLAGS := -std=gnu99 -Wall -Wextra -Werror -Wshadow -Wpointer-arith \
          -Wstrict-prototypes -Wmissing-prototypes \
					-I$(INCLUDEDIR) -I$(LUAJIT_DIR)
LDFLAGS :=
PROGNAME := brainfuck

SOURCES := $(shell find src/ -name "*.c")
DASM_SOURCES := $(shell find src/ -name "*.dasm")
OBJECTS := $(SOURCES:.c=.o)
DASM_OBJECTS := $(DASM_SOURCES:.dasm=.o)

jit: CFLAGS += -DFEATURE_JIT -O2
jit: SOURCES += $(DASM_SOURCES)
jit: OBJECTS += $(DASM_OBJECTS)
jit: $(MINILUA) $(OBJECTS) $(JIT_O)
jit:
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(PROGNAME)

jit_debug: CFLAGS += -DFEATURE_JIT -g -DDEBUG
jit_debug: SOURCES += $(DASM_SOURCES)
jit_debug: OBJECTS += $(DASM_OBJECTS)
jit_debug: $(MINILUA) $(OBJECTS) $(JIT_O)
jit_debug:
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(PROGNAME)

interpreter: CFLAGS += -O2
interpreter: $(PROGNAME)

interpreter_debug: CFLAGS += -g -DDEBUG
interpreter_debug: $(PROGNAME)

$(MINILUA):
	$(CC) $(LUAJIT_DIR)/src/host/minilua.c -lm -o $(MINILUA)

$(PROGNAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(PROGNAME)

$(JIT_O): $(MINILUA) $(JIT_DASM)
	./$(MINILUA) luajit-2.0/dynasm/dynasm.lua -o $(JIT_C) -D X64 $(JIT_DASM)
	$(CC) $(CFLAGS) -c -o $(JIT_O) $(JIT_C)
	rm -f $(JIT_C)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(MINILUA)
	rm -f $(PROGNAME)
	rm -f $(OBJECTS)
	rm -f $(JIT_O)
	rm -f $(JIT_C)
	rm -f $(shell find . -name *~)

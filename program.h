#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

enum brainfuck_op_type {
  PLUS,
  MINUS,
  SHIFT_LEFT,
  SHIFT_RIGHT,
  BRANCH_LEFT,
  BRANCH_RIGHT,
  INPUT,
  OUTPUT,
  EXIT,
  ZERO,
  NOP
};

struct brainfuck_op {
  enum brainfuck_op_type type;
  int op_value;
};

struct program {
  struct brainfuck_op* ops;
  size_t length;
  size_t capacity;
};

void debug_dump_program(struct program *prog);

// Reads a brainfuck program from the input stream.
// Does not attempt to do any checking - just globs things
// from the input stream and returns a string of symbols.
struct program read_program(FILE *stream);

// Optimizes series of the same opcode so that the opcode
// is performed once. This pass optimizes things such as
// ++++++++ into +(8), which is one instruction that adds 8
// to the current cell instead of 8 instructions that add
// one.
struct program aggregate_ops(struct program *prog);

// Populates the branch offset field of the brainfuck_op and checks
// that all branches are balanced, returning false if they are not.
bool mark_branches(struct program *prog);

// Optimizes instruction patterns of the form [-] to simply set the
// value at the memory pointer to zero.
void zero_cell_optimization(struct program *prog);

void destroy_program(struct program *prog);

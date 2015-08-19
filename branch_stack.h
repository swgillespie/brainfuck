#pragma once

#include "program.h"

struct branch_stack_entry {
  size_t branch_offset;
  struct brainfuck_op *op;
};

struct branch_stack {
  struct branch_stack_entry *stack;
  size_t length;
  size_t capacity;
};

struct branch_stack branch_stack_new(void);
void branch_stack_push(struct branch_stack *stack, size_t offset, struct brainfuck_op *op);
struct branch_stack_entry branch_stack_pop(struct branch_stack *stack);
void branch_stack_destroy(struct branch_stack *stack);

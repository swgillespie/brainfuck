#include <stdlib.h>
#include "branch_stack.h"

#define BRANCH_STACK_DEFAULT_SIZE 256
#define BRANCH_STACK_SCALING_FACTOR 2

static void
branch_stack_resize(struct branch_stack *stack) {
  stack->capacity = stack->capacity * BRANCH_STACK_SCALING_FACTOR;
  stack->stack = realloc(stack->stack, sizeof(struct branch_stack_entry) * stack->capacity);
}

struct branch_stack
branch_stack_new() {
  struct branch_stack stack;
  stack.stack = calloc(sizeof(struct branch_stack_entry), BRANCH_STACK_DEFAULT_SIZE);
  stack.length = 0;
  stack.capacity = BRANCH_STACK_DEFAULT_SIZE;
  return stack;
}

void
branch_stack_push(
  struct branch_stack *stack,
  size_t offset,
  struct brainfuck_op *op) {
    if (stack->length == stack->capacity) {
      branch_stack_resize(stack);
    }
    struct branch_stack_entry entry;
    entry.branch_offset = offset;
    entry.op = op;
    stack->stack[stack->length++] = entry;
}

struct branch_stack_entry
branch_stack_pop(struct branch_stack *stack) {
  if (stack->length == 0) {
    fprintf(stderr, "attempted to pop from empty branch stack");
    abort();
  }

  return stack->stack[--stack->length];
}

void
branch_stack_destroy(struct branch_stack *stack) {
  free(stack->stack);
}

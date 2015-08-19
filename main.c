#include <stdio.h>
#include <string.h>
#include "program.h"

#define BRANCH_STACK_MAX 256
#define MEMORY_SIZE 30000

size_t find_next_branch_right(struct program*, size_t);
void execute(struct program);

size_t
find_next_branch_right(struct program *prog, size_t pc) {
  for (size_t i = pc; i < prog->length; i++) {
    if (prog->ops[i].type == BRANCH_RIGHT) {
      return i;
    }
  }

  // if the program is valid, this branch will not be taken.
  printf("Attempted to execute a non-valid program. Aborting\n");
  abort();
}

void
execute(struct program prog) {
  static void *dispatch_table[] = {
    &&plus,
    &&minus,
    &&shift_left,
    &&shift_right,
    &&branch_left,
    &&branch_right,
    &&input,
    &&output,
    &&exit,
    &&zero,
    &&nop
  };
  int op_value;

  #define DISPATCH(prog, pc) do {                   \
    size_t new_pc = (pc);                           \
    op_value = (prog).ops[new_pc].op_value;         \
    goto *dispatch_table[(prog).ops[new_pc].type];  \
  } while(0)

  size_t pc = 0;
  char *memory_ptr = alloca(MEMORY_SIZE);
  memset(memory_ptr, 0, MEMORY_SIZE);
  DISPATCH(prog, pc);
plus:
  *memory_ptr += op_value;
  DISPATCH(prog, ++pc);
minus:
  *memory_ptr -= op_value;
  DISPATCH(prog, ++pc);
shift_left:
  memory_ptr -= op_value;
  DISPATCH(prog, ++pc);
shift_right:
  memory_ptr += op_value;
  DISPATCH(prog, ++pc);
branch_left:
  if (*memory_ptr == 0) {
    pc = op_value;
  }
  DISPATCH(prog, ++pc);
branch_right:
  if (*memory_ptr != 0) {
    pc = op_value;
  }
  DISPATCH(prog, ++pc);
output:
  for (int i = 0; i < op_value; i++) {
    putchar(*memory_ptr);
    fflush(stdout);
  }
  DISPATCH(prog, ++pc);
input:
  for (int i = 0; i < op_value; i++) {
    *memory_ptr = getchar();
  }
  DISPATCH(prog, ++pc);
exit:
  destroy_program(&prog);
  return;
zero:
  *memory_ptr = 0;
  DISPATCH(prog, ++pc);
nop:
  DISPATCH(prog, ++pc);

  #undef DISPATCH
}

int
main(int argc, char **argv)
{
  // first - read the program from standard in
  // struct program prog = read_program(stdin);
  if (argc != 2) {
    printf("usage: %s <filename>\n", argv[0]);
    exit(1);
  }
  FILE *f = fopen(argv[1], "r");
  if (f == NULL) {
    perror("failed to open file");
    exit(1);
  }

  struct program prog = read_program(f);
  prog = aggregate_ops(&prog);
  if (!mark_branches(&prog)) {
    printf("Invalid branches\n");
    exit(1);
  }

  zero_cell_optimization(&prog);

  // next, execute it.
  execute(prog);
  fclose(f);
}

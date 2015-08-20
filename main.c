#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include "program.h"

#define BRANCH_STACK_MAX 256
#define MEMORY_SIZE 30000

void execute(struct program);

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
    &&nop,
    &&move_left,
    &&move_right,
    &&scan
  };

  #define DISPATCH(prog, ip) goto *dispatch_table[(ip)->type]
  
  struct brainfuck_op *ip = prog.ops;
  int *memory_ptr = alloca(sizeof(int) * MEMORY_SIZE);
  memset(memory_ptr, 0, MEMORY_SIZE);
  DISPATCH(prog, ip);
plus:
  *memory_ptr += ip->op_value;
  DISPATCH(prog, ++ip);
minus:
  *memory_ptr -= ip->op_value;
  DISPATCH(prog, ++ip);
shift_left:
  memory_ptr -= ip->op_value;
  DISPATCH(prog, ++ip);
shift_right:
  memory_ptr += ip->op_value;
  DISPATCH(prog, ++ip);
branch_left:
  if (*memory_ptr == 0) {
    ip += ip->op_value;
  }
  DISPATCH(prog, ++ip);
branch_right:
  if (*memory_ptr != 0) {
    ip += ip->op_value;
  }
  DISPATCH(prog, ++ip);
output:
  for (int i = 0; i < ip->op_value; i++) {
    putchar(*memory_ptr);
    fflush(stdout);
  }
  DISPATCH(prog, ++ip);
input:
  for (int i = 0; i < ip->op_value; i++) {
    *memory_ptr = getchar();
  }
  DISPATCH(prog, ++ip);
exit:
  destroy_program(&prog);
  return;
zero:
  *memory_ptr = 0;
  DISPATCH(prog, ++ip);
nop:
  DISPATCH(prog, ++ip);
move_left:
  *(memory_ptr - ip->op_value) += *memory_ptr;
  *memory_ptr = 0;
  DISPATCH(prog, ++ip);
move_right:
  *(memory_ptr + ip->op_value) += *memory_ptr;
  *memory_ptr = 0;
  DISPATCH(prog, ++ip);
scan:
  while (*memory_ptr != 0) memory_ptr += ip->op_value;
  DISPATCH(prog, ++ip);

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
  move_gadget_detection(&prog);
  scan_gadget_detection(&prog);

  // next, execute it.
  execute(prog);

  printf("\n");
  printf("ops combined: %d\n", OPS_AGGREGATED);
  printf("zero ops eliminated: %d\n", ZERO_OPS_ELIMINATED);
  printf("move ops eliminated: %d\n", MOVE_OPS_ELIMINATED);
  printf("scan ops eliminated: %d\n", SCAN_OPS_ELIMINATED);
  fclose(f);
}

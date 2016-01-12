#include "program.h"
#include "branch_stack.h"

#define INITIAL_PROGRAM_SIZE 1 << 8
#define SCALING_FACTOR 2
#define BRANCH_STACK_SIZE 256

int OPS_AGGREGATED = 0;
int ZERO_OPS_ELIMINATED = 0;
int MOVE_OPS_ELIMINATED = 0;
int SCAN_OPS_ELIMINATED = 0;

void
debug_dump_program(struct program *prog) {
  for (size_t i = 0; i < prog->length; i++) {
    struct brainfuck_op op = prog->ops[i];
    switch(op.type) {
      case PLUS:
        if (op.op_value != 0)
          printf("+(%d)", op.op_value);
        else
          printf("+");
        break;
      case MINUS:
        if (op.op_value != 0)
          printf("-(%d)", op.op_value);
        else
          printf("-");
        break;
      case SHIFT_LEFT:
        if (op.op_value != 0)
          printf("<(%d)", op.op_value);
        else
          printf("<");
        break;
      case SHIFT_RIGHT:
        if (op.op_value != 0)
          printf(">(%d)", op.op_value);
        else
          printf(">");
        break;
      case BRANCH_LEFT:
        if (op.op_value != 0)
          printf("[(%d)", op.op_value);
        else
          printf("[");
        break;
      case BRANCH_RIGHT:
        if (op.op_value != 0)
          printf("](%d)", op.op_value);
        else
          printf("]");
        break;
      case INPUT:
        if (op.op_value != 0)
          printf(",(%d)", op.op_value);
        else
          printf(",");
        break;
      case OUTPUT:
        if (op.op_value != 0)
          printf(".(%d)", op.op_value);
        else
          printf(".");
        break;
      case EXIT:
        printf("<END>");
        break;
      case NOP:
        printf("<NOP>");
        break;
      case ZERO:
        printf("<ZERO>");
        break;
      case MOVE_LEFT:
        printf("<MOVE LEFT %d>", op.op_value);
        break;
      case MOVE_RIGHT:
        printf("<MOVE RIGHT %d>", op.op_value);
        break;
      case SCAN:
        printf("<SCAN %d>", op.op_value);
        break;
    }
  }
  printf("\n");
}

static void
resize_program(struct program *prog) {
  prog->capacity = prog->capacity * SCALING_FACTOR;
  prog->ops = realloc(prog->ops, sizeof(struct brainfuck_op) * prog->capacity);
}

static inline void
insert_op(struct program *prog, struct brainfuck_op op) {
  if (prog->capacity == prog->length) {
    resize_program(prog);
  }

  prog->ops[prog->length++] = op;
}

struct program
read_program(FILE *stream) {
  struct program scanned_program;
  scanned_program.ops = calloc(sizeof(struct brainfuck_op), INITIAL_PROGRAM_SIZE);
  scanned_program.length = 0;
  scanned_program.capacity = INITIAL_PROGRAM_SIZE;
  int scanned_char;
  while ((scanned_char = fgetc(stream)) != EOF) {
    struct brainfuck_op op;
    op.op_value = 0;
    switch (scanned_char) {
      case '+':
        op.type = PLUS;
        insert_op(&scanned_program, op);
        break;
      case '-':
        op.type = MINUS;
        insert_op(&scanned_program, op);
        break;
      case '<':
        op.type = SHIFT_LEFT;
        insert_op(&scanned_program, op);
        break;
      case '>':
        op.type = SHIFT_RIGHT;
        insert_op(&scanned_program, op);
        break;
      case '[':
        op.type = BRANCH_LEFT;
        insert_op(&scanned_program, op);
        break;
      case ']':
        op.type = BRANCH_RIGHT;
        insert_op(&scanned_program, op);
        break;
      case '.':
        op.type = OUTPUT;
        insert_op(&scanned_program, op);
        break;
      case ',':
        op.type = INPUT;
        insert_op(&scanned_program, op);
        break;
    }
  }

  struct brainfuck_op op;
  op.type = EXIT;
  op.op_value = 0;
  insert_op(&scanned_program, op);
  return scanned_program;
}

struct program
aggregate_ops(struct program *prog) {
  struct program aggregated_program;
  aggregated_program.ops = calloc(sizeof(struct brainfuck_op), INITIAL_PROGRAM_SIZE);
  aggregated_program.length = 0;
  aggregated_program.capacity = INITIAL_PROGRAM_SIZE;

  enum brainfuck_op_type last_seen_type = NOP;
  int count = 1;
  for (size_t i = 0; i < prog->length; i++) {
    if (prog->ops[i].type == last_seen_type) {
      // if the type we're looking at is the same as the last one,
      // increase the count and continue.
      if (last_seen_type == BRANCH_LEFT || last_seen_type == BRANCH_RIGHT) {
        // if the previous type was a branch and this one was as well, do not
        // combine them.
        struct brainfuck_op op;
        op.type = last_seen_type;
        op.op_value = 0;
        insert_op(&aggregated_program, op);
        continue;
      }

      OPS_AGGREGATED++;
      count++;
    } else {
      // if we're looking at a type that's not the same as the last type
      // that we observed, we need to "finish" off the last type.
      struct brainfuck_op op;
      if (last_seen_type != NOP) {
        op.type = last_seen_type;
        op.op_value = count;
        insert_op(&aggregated_program, op);
      }
      last_seen_type = prog->ops[i].type;
      count = 1;
    }
  }

  if (count != 1) {
    struct brainfuck_op op;
    op.type = last_seen_type;
    op.op_value = count;
    insert_op(&aggregated_program, op);
  }

  // stick a terminator onto the new program
  struct brainfuck_op exit_op;
  exit_op.type = EXIT;
  exit_op.op_value = 0;
  insert_op(&aggregated_program, exit_op);

  destroy_program(prog);
  return aggregated_program;
}

bool
mark_branches(struct program *prog) {
  struct stack_entry {
    size_t branch_offset;
    struct brainfuck_op *op;
  };

  struct branch_stack stack = branch_stack_new();
  for (size_t i = 0; i < prog->length; i++) {
    struct brainfuck_op *op = &prog->ops[i];
    switch (op->type) {
      case BRANCH_LEFT:
        branch_stack_push(&stack, i, op);
        break;
      case BRANCH_RIGHT:
        if (stack.length == 0) {
          branch_stack_destroy(&stack);
          return false;
        }

        struct branch_stack_entry source_branch = branch_stack_pop(&stack);
        // for this branch_right, we want the branch offset to point to
        // the next instruction after the branch_left that is at the top
        // of the stack.
        op->op_value = source_branch.branch_offset - i;
        // we also want the branch_left to point to the instruction
        // after this branch_right
        source_branch.op->op_value = i - source_branch.branch_offset;
        break;
      default:
        continue;
    }
  }

  branch_stack_destroy(&stack);
  return true;
}

void
zero_cell_optimization(struct program *prog) {
  for (size_t i = 1; i < prog->length - 1; i++) {
    if (prog->ops[i-1].type == BRANCH_LEFT &&
        prog->ops[i].type == MINUS &&
        prog->ops[i+1].type == BRANCH_RIGHT) {
          ZERO_OPS_ELIMINATED++;
          prog->ops[i-1].type = NOP;
          prog->ops[i].type = ZERO;
          prog->ops[i+1].type = NOP;
        }
  }
}

void
move_gadget_detection(struct program *prog) {
  for (size_t i = 2; i < prog->length - 3; i++) {
    // i - 2 = [
    // i - 1 = -
    // i = < or >
    // i + 1 = +
    // i + 2 = > or <
    // i + 3 = ]
    if (prog->ops[i-2].type == BRANCH_LEFT &&
        prog->ops[i-1].type == MINUS &&
        (prog->ops[i].type == SHIFT_LEFT || prog->ops[i].type == SHIFT_RIGHT) &&
        prog->ops[i+1].type == PLUS &&
        (prog->ops[i+2].type == SHIFT_LEFT || prog->ops[i+2].type == SHIFT_RIGHT) &&
        prog->ops[i+3].type == BRANCH_RIGHT) {
          // couple things to check before we actually do this optimization
          // ops i and i+2 must be opposite and they must be the same number
          if (prog->ops[i].type == prog->ops[i+2].type) {
            continue;
          }

          if (prog->ops[i].op_value != prog->ops[i+2].op_value) {
            continue;
          }

          // otherwise - let's do it.
          MOVE_OPS_ELIMINATED++;
          prog->ops[i-2].type = NOP;
          prog->ops[i-1].type = NOP;
          prog->ops[i].type = prog->ops[i].type == SHIFT_LEFT ? MOVE_LEFT : MOVE_RIGHT;
          prog->ops[i+1].type = NOP;
          prog->ops[i+2].type = NOP;
          prog->ops[i+3].type = NOP;
        }
  }
}

void
scan_gadget_detection(struct program *prog) {
  for (size_t i = 1; i < prog->length - 1; i++) {
    if (prog->ops[i-1].type == BRANCH_LEFT &&
        (prog->ops[i].type == SHIFT_LEFT || prog->ops[i].type == SHIFT_RIGHT) &&
        prog->ops[i+1].type == BRANCH_RIGHT) {
          SCAN_OPS_ELIMINATED++;
          prog->ops[i-1].type = NOP;
          if (prog->ops[i].type == SHIFT_LEFT) {
            prog->ops[i].op_value *= -1;
          }
          prog->ops[i].type = SCAN;
          prog->ops[i+1].type = NOP;
        }
  }
}

void destroy_program(struct program *prog) {
  free(prog->ops);
}

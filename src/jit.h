#pragma once

#include <stdbool.h>
#include "program.h"


#ifdef FEATURE_JIT
# ifndef __x86_64__
#  error("FEATURE_JIT only enabled on x64")
# endif // __X86_64__

typedef void(*bf_main_func)(char* memory);

struct bf_func {
  bf_main_func func;
  size_t size;
};

// Compiles a brainfuck program into executable code, returning
// a pointer to the compiled function.
struct bf_func jit_compile(struct program* prog);

// Frees a JIT-compiled function.
void jit_free(struct bf_func* func);
#endif // FEATURE_JIT

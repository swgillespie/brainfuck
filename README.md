This is a quick and dirty Brainfuck interpreter and JIT compiler.

## Building and Running
This interpreter uses DynASM, a component of LuaJIT. LuaJIT is hosted as a submodule
in this repo, so closing recursively will obtain it:
```
git clone --recursive https://github.com/swgillespie/brainfuck.git
```

After that, `brainfuck` builds with GNU make:
```
make
```

The makefile supports several kinds of builds:
* `make jit`, the default build, which builds the engine with the jit and with optimizations
* `make jit_debug`, the jit build but without optimizations and debug info
* `make interpreter`, build the engine with an interpreter and with optimizations
* `make interpreter_debug`, the interpreter build without optimizations and debug info.

The JIT compiler is significantly faster than the interpreter so there's no reason to use it
unless you're on a non-x64 system. The interpreter is platform-independent but makes use of
non-standard C, in particular a GNU extension allows taking labels as values and jumping to them
with `goto`. `clang` and `gcc` both handle this fine (clang produces a warning) but I'm sure
`msvc` does not. The JIT is very *not* platform independent so it's only enabled on x64 for now.

It is undefined behavior for a brainfuck program to use memory outside the alloted tape. This usually
results in segfaults and can be confirmed using valgrind. The default memory size is pretty huge but
I've still run into problems (in particular, asking a brainfuck compiler to compile the Lost Kingdom
program results in high memory usage. shocker.)

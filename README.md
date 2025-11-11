# CS333 Project 5

This repository contains starter notes for CS 333 Programming Languages Project 5. The project focuses on:

- Signal handling and error recovery examples in C.
- File I/O and word frequency counting using linked lists.
- Experiments measuring memory allocation performance in C.
- A simulated mark-and-sweep garbage collector exercise.
- Comparative investigations of file I/O and memory management in another language of choice.

Use this repository as a workspace for tracking your implementations, documentation, and supporting materials.

## Repository layout

The repository now includes starter C programs for each major task in Part I of the assignment. They are grouped by topic:

- `c/signals/`
  - `sigint_example.c` – installs a `SIGINT` handler that exits cleanly after printing a message.
  - `sigfpe_example.c` – demonstrates recovering from a floating-point exception via `sigsetjmp`/`siglongjmp`.
  - `sigsegv_example.c` – catches `SIGSEGV`, reports the fault, and terminates safely.
- `c/word_counter/word_counter.c` – a case-insensitive word-frequency counter that ignores punctuation and prints the top 20 entries.
- `c/memory_timing/malloc_timing.c` – measures the average `malloc` time for multiple allocation sizes while discarding the first warm-up call.
- `c/gc_sim/mark_sweep.c` – a mark-and-sweep simulator with stack/heap structures and an example containing an unreachable reference cycle.

Each source file is self-contained and can be compiled with `gcc`. Example commands (from the repository root):

```bash
mkdir -p build
gcc -Wall -Wextra -std=c17 -o build/sigint c/signals/sigint_example.c
gcc -Wall -Wextra -std=c17 -o build/word_counter c/word_counter/word_counter.c
```

Feel free to adapt these examples as you develop the final implementations, add screenshots, and document results in the project report.

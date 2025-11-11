# CS333 Project 5
### Max Petite
### 11/13/2025

**Google Sites Report:** https://sites.google.com/colby.edu/maxs-cs333/project5

## Directory Layout
```
Project5_MaxPetite/
├── README.md
├── detect_garbage.c
├── wctest.txt
├── c/
│   ├── signals/
│   │   ├── sigfpe_example.c
│   │   ├── sigint_example.c
│   │   └── sigsegv_example.c
│   ├── memory_timing/
│   │   └── malloc_timing.c
│   ├── gc_sim/
│   │   └── mark_sweep.c
│   └── word_counter/
│       └── word_counter.c
```

## Environment Notes
- OS: Linux 6.12.13 (x86_64)
- C compiler: gcc (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0

All terminal transcripts below capture the exact build commands, runtime invocations, and program output in lieu of screenshots.

## Part I — File I/O and Memory Management in C

### Task 1 — POSIX Signal Handling

#### SIGINT Handler
##### Build
```bash
$ gcc -std=c11 -Wall -Wextra -pedantic c/signals/sigint_example.c -o sigint_example
```
##### Run
```bash
$ ./sigint_example
Press Ctrl+C to trigger SIGINT (Ctrl+C)...
^C
Interrupted! Exiting.
```
##### Implementation / Testing Notes
- The handler writes its message with `write(2)` and terminates via `_exit` so that the routine stays async-signal-safe while exiting immediately after the interrupt.【F:c/signals/sigint_example.c†L6-L24】

#### SIGFPE Handler
##### Build
```bash
$ gcc -std=c11 -Wall -Wextra -pedantic c/signals/sigfpe_example.c -o sigfpe_example
```
##### Run
```bash
$ ./sigfpe_example
Attempting to divide by zero to trigger SIGFPE...
Caught SIGFPE (floating point exception). Recovering...
Attempting to divide by zero to trigger SIGFPE...
Floating point exception
```
##### Implementation / Testing Notes
- `sigsetjmp`/`siglongjmp` allow the program to recover from the first divide-by-zero fault, increment a fault counter, and continue execution before letting the second fault terminate the process, demonstrating both recovery and eventual failure handling.【F:c/signals/sigfpe_example.c†L8-L41】

#### SIGSEGV Handler
##### Build
```bash
$ gcc -std=c11 -Wall -Wextra -pedantic c/signals/sigsegv_example.c -o sigsegv_example
```
##### Run
```bash
$ ./sigsegv_example
Deliberately causing a segmentation fault...
Segmentation fault caught! Cleaning up and exiting.
```
##### Implementation / Testing Notes
- The handler logs to `stderr` using `write(2)` and terminates with `_exit`, ensuring the message is emitted even though the main thread dereferences a null pointer to trigger `SIGSEGV`.【F:c/signals/sigsegv_example.c†L6-L26】

### Task 2 — Word Frequency Counter (Linked List)
##### Build
```bash
$ gcc -std=c11 -Wall -Wextra -pedantic c/word_counter/word_counter.c -o word_counter
```
##### Run
```bash
$ ./word_counter wctest.txt
Top 20 words by frequency:
the             17
of              7
and             6
was             5
in              4
with            4
a               3
windows         3
been            2
broken          2
but             2
central         2
had             2
portion         2
stone           2
up              2
were            2
wings           2
against         1
any             1
```
##### Implementation / Testing Notes
- Tokens are normalized to lowercase and stripped of punctuation while the linked-list accumulator increments counts in-place for repeated words, satisfying the case-insensitive, punctuation-agnostic requirements.【F:c/word_counter/word_counter.c†L15-L181】
- The list is flattened into an array, sorted by frequency, and the top 20 entries are printed to match the expected grading output.【F:c/word_counter/word_counter.c†L149-L187】

### Task 3 — Allocation Timing Study
##### Build
```bash
$ gcc -std=c11 -Wall -Wextra -pedantic c/memory_timing/malloc_timing.c -o malloc_timing
```
##### Run
```bash
$ ./malloc_timing
Average allocation time per malloc call:
Scenario        Iterations      Average seconds
100 ints          50000 0.000000022
10K ints          10000 0.000000026
1M ints             500 0.000000391
```
##### Implementation / Testing Notes
- Each scenario performs a warm-up allocation, then times repeated `malloc` calls with `clock_gettime`, averaging the elapsed seconds after freeing the buffers to highlight the cost scaling from 100 integers to one million integers.【F:c/memory_timing/malloc_timing.c†L19-L75】

### Task 4 — Mark-and-Sweep Garbage Detection
##### Build
```bash
$ gcc -std=c11 -Wall -Wextra -pedantic detect_garbage.c -o detect_garbage
```
##### Run
```bash
$ ./detect_garbage
HeapChunk[0] @ 0x55f477a7b3e0 is reachable.
HeapChunk[1] @ 0x55f477a7b420 is reachable.
HeapChunk[2] @ 0x55f477a7b4a0 is garbage.
HeapChunk[3] @ 0x55f477a7b4e0 is reachable.
```
##### Implementation / Testing Notes
- The helper `duplicateString` replaces the POSIX `strdup` dependency so stack variable names are safely copied without feature-test macros.【F:detect_garbage.c†L50-L59】【F:detect_garbage.c†L72-L85】
- A recursive depth-first traversal marks every heap chunk reachable from the simulated stack before the reporting loop labels the unreachable entry as garbage, even in the presence of cycles between `a` and `b`.【F:detect_garbage.c†L110-L144】

## Part II — Selected Language Research
See the linked Google Sites report for language-specific error handling, file I/O, and memory management write-ups, including the replicated word-frequency tool and garbage-collection timing experiment.

## Extensions
- None for this submission.

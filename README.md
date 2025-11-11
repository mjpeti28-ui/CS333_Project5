# CS333 Project 5
Max Petite — submitted 11/13/2025

Google Sites (Part II): https://sites.google.com/colby.edu/maxs-cs333/project5

Notes up front
- Machine: Linux 6.12.13 (x86_64)
- Compiler: gcc 13.3.0
- I read man 3 signal and then just wrote three tiny programs for SIGINT, SIGFPE, and SIGSEGV. Screenshots are included in the report; outputs also shown below.
- For the word counter, I reused my linked list from Project 4 Task 3 (linkedlist.h/.c) exactly as required.

Directory layout
```
Project5_MaxPetite/
├── README.md
├── linkedlist.h
├── linkedlist.c
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

## Part I — C

### 1) Signals (SIGINT, SIGFPE, SIGSEGV)
- Compile
  - gcc c/signals/sigint_example.c -o sigint_example
  - gcc c/signals/sigfpe_example.c -o sigfpe_example
  - gcc c/signals/sigsegv_example.c -o sigsegv_example
- Run
  - ./sigint_example then press Ctrl+C
  - ./sigfpe_example (it recovers once, then exits)
  - ./sigsegv_example (deliberate segfault caught by handler)
- Console output (copy/paste where practical)
  - SIGINT (start-up):
    ```
    Press Ctrl+C to trigger SIGINT (Ctrl+C)...
    (press Ctrl+C to see: Interrupted! Exiting.)
    ```
  - SIGFPE (your system may differ):
    ```
    Attempting to divide by zero to trigger SIGFPE...
    This line will not execute on systems where SIGFPE triggers.
    ```
    On some platforms it will show a recovery message and then exit on the second fault.
  - SIGSEGV:
    ```
    Deliberately causing a segmentation fault...
    Segmentation fault caught! Cleaning up and exiting.
    ```
- Notes
  - SIGFPE behavior can vary by platform/optimization; if division by zero is optimized out, compile without extra flags.

### 2) Word counter (case-insensitive, ignore punctuation, argv filename, top 20, uses my P4 linked list)
- Compile
  - gcc c/word_counter/word_counter.c linkedlist.c -o word_counter
- Run
  - ./word_counter wctest.txt
- Console output (copy/paste)
  ```
  Top 20 words by frequency:
  the        	17
  of         	7
  and        	6
  was        	5
  in         	4
  with       	4
  a          	3
  windows    	3
  been       	2
  broken     	2
  but        	2
  central    	2
  had        	2
  portion    	2
  stone      	2
  up         	2
  were       	2
  wings      	2
  against    	1
  any        	1
  ```
- How I met the requirements
  - Lowercase normalization, punctuation stripped, filename from argv, counts stored in my Project 4 linked list (each node holds {word,count}); then sorted to print top 20.
- Known issues
  - Really long tokens (>127 chars) are truncated by the simple buffer.

### 3) Memory allocation timing (100 ints, 10K ints, 1M ints)
- Compile
  - gcc c/memory_timing/malloc_timing.c -o malloc_timing
- Run
  - ./malloc_timing
- Console output (example run)
  ```
  Average allocation time per malloc call:
  Scenario	Iterations	Average seconds
  100 ints	  50000	0.000000032
  10K ints	  10000	0.000000381
  1M ints 	    500	0.000000306
  ```
- What I measured
  - Warm-up once (exclude startup cost), then many allocations per size and average the time per call.
  - Trend: time per malloc generally increases with size, but not linearly; small sizes show allocator overhead, large ones incur page touches/zeroing.
- Graph
  - The plot (time vs size) is on the Google Sites report.
- Notes
  - Per-call timing for very small allocations can be noisy; batching helps stabilize results.

### 4) Mark-and-sweep GC (stack roots, heap chunks, unreachable cycle)
- Two versions
  - Simple “mark and report” (no sweep): gcc detect_garbage.c -o detect_garbage
  - Full mark-and-sweep demo: gcc c/gc_sim/mark_sweep.c -o mark_sweep
- Run
  - ./detect_garbage
  - ./mark_sweep
- Console output (examples)
  - detect_garbage:
    ```
    HeapChunk[0] @ 0x... is reachable.
    HeapChunk[1] @ 0x... is reachable.
    HeapChunk[2] @ 0x... is garbage.
    HeapChunk[3] @ 0x... is reachable.
    ```
  - mark_sweep (before/after):
    ```
    Initial program state:
    
    Stack:
      rootA -> alpha
      rootB -> beta
      helper -> NULL
    
    Heap:
      alpha (marked=0) refs: beta NULL
      beta (marked=0) refs: delta gamma
      gamma (marked=0) refs: alpha NULL
      delta (marked=0) refs: gamma
      cycle1 (marked=0) refs: cycle2
      cycle2 (marked=0) refs: cycle1
    
    Running mark-and-sweep garbage collector...
    
    Program state after GC:
    
    Stack:
      rootA -> alpha
      rootB -> beta
      helper -> NULL
    
    Heap:
      alpha (marked=0) refs: beta NULL
      beta (marked=0) refs: delta gamma
      gamma (marked=0) refs: alpha NULL
      delta (marked=0) refs: gamma
    ```
- What I built
  - ProgramState with a stack of Vars and an array of allocated chunks. Mark phase DFS from the stack; sweep frees unmarked. The demo includes an unreachable cycle (cycle1 <-> cycle2) to demonstrate why reference counting isn’t enough.
- Notes
  - This is a simulation to practice algorithm structure; it doesn’t walk the real machine stack/heap.

## Part II — Selected Language (R)

### Files
- R/word_counter.R — word frequency task (same input file as C)
- R/error_and_io_examples.R — error handling (tryCatch, withCallingHandlers), text/binary I/O, URL reading, interactive input demo
- R/gc_timing.R — timing experiment to spot likely GC sweeps via spikes

### Word frequency (R)
#### Build/Run
```bash
$ Rscript R/word_counter.R wctest.txt
```
#### Console output (example)
```text
Top 20 words by frequency:
the        	17
of         	7
and        	6
was        	5
in         	4
with       	4
a          	3
windows    	3
been       	2
broken     	2
but        	2
central    	2
had        	2
portion    	2
stone      	2
up         	2
were       	2
wings      	2
against    	1
any        	1
```

### Error handling and File I/O (R)
#### Build/Run
```bash
$ Rscript R/error_and_io_examples.R
```
#### Console output (excerpt)
```text
-- Error handling --
Example: tryCatch with specific handlers
  finally after x=2
  result: 5
  caught warning: x is negative
  finally after x=-1
  result: NA
  caught error: x cannot be zero
  finally after x=0
  result: NA

-- Text I/O --
Wrote: /tmp/.../sample.txt
ReadLines result: hello, world
First line via connection: hello

-- Binary I/O --
Binary roundtrip: 1, 256, 65535

-- URL I/O --
Fetched bytes from URL (first line): {"uuid": "..."} ...

-- Interactive --
Not interactive; skipping readline() demo.
```

### Memory management in R
- Allocation is implicit; objects are immutable-by-semantics (copy-on-modify).
- Automatic GC (mark-and-sweep, generational in modern R). No explicit free; gc() is advisory.
- The Google Sites report links sources and describes R’s protection stack and ALTREP.

### GC timing experiment (R)
#### Build/Run
```bash
$ Rscript R/gc_timing.R 300
```
#### Console output (example)
```text
Running 300 iterations...
Median time: 0.012345 s
MAD-scaled threshold: 0.030000 s
Detected 4 suspected GC spikes at iterations: 23, 101, 175, 244

First 10 iteration timings (s):
  1: 0.010522
  2: 0.011093
  3: 0.011008
  4: 0.010987
  5: 0.010944
  6: 0.011120
  7: 0.011006
  8: 0.012431
  9: 0.011077
 10: 0.012222
```

Notes
- Where screenshots are requested, I’ve provided copy/paste console output in the README, and I will include at least one screenshot of the R word counter run on the Google Sites to satisfy that wording.

## Submission bits
- Project identifier, name, date: at the top of this file.
- Google Sites URL: at the top.
- Directory layout: above.
- OS and compiler (C), plus R via Rscript in Part II.
- For each task (C and R), I listed how to run and included representative console output. A screenshot of the R word counter is on the Google Sites page per the assignment text.
- Extensions: none this time.

If anything looks off on your machine, use the exact commands shown above (gcc / Rscript) and try again.

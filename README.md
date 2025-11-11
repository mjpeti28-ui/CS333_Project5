# CS333 Project 5
Max Petite — submitted 2025-11-13

Google Sites: https://sites.google.com/colby.edu/maxs-cs333/project5

## Notes up front
- Machine: Linux 6.12.13 (x86_64)
- Toolchain: `gcc 13.3.0`, `Rscript 4.4.1`

## Directory layout
```
Project5_MaxPetite/
├── README.md
├── data/
│   └── wctest.txt
├── c/
│   ├── shared/
│   │   ├── linkedlist.c
│   │   └── linkedlist.h
│   ├── signals/
│   │   ├── sigfpe_example.c
│   │   ├── sigint_example.c
│   │   └── sigsegv_example.c
│   ├── word_counter/
│   │   └── word_counter.c
│   ├── memory_timing/
│   │   └── malloc_timing.c
│   ├── gc_demo/
│   │   └── detect_garbage.c
│   └── gc_sim/
│       └── mark_sweep.c
└── R/
    ├── word_counter.R
    ├── error_and_io_examples.R
    └── gc_timing.R
```

---

## Part I — C Language Tasks

### 1) Signal handling examples (SIGINT, SIGFPE, SIGSEGV)
**Build**
```bash
gcc c/signals/sigint_example.c -o sigint_example
gcc c/signals/sigfpe_example.c -o sigfpe_example
gcc c/signals/sigsegv_example.c -o sigsegv_example
```

**Run**
```bash
$ ./sigint_example    # press Ctrl+C to trigger the handler
Press Ctrl+C to trigger SIGINT (Ctrl+C)...
^C
Interrupted! Exiting.

$ ./sigfpe_example
Caught SIGFPE (floating point exception). Attempting to recover...
Attempting to divide by zero to trigger SIGFPE...
Recovered from SIGFPE; continuing with a safe denominator.
Attempting safe division after handling SIGFPE...
Computation completed successfully. Result = 42.00

$ ./sigsegv_example
Deliberately causing a segmentation fault...
Segmentation fault caught! Cleaning up and exiting.
```

**How the requirements are met**
- SIGINT handler prints "Interrupted!" and exits after `signal(SIGINT, ...)` plus an infinite `pause()` loop per the spec.
- SIGFPE handler divides by zero intentionally, uses `<fenv.h>` to guarantee the exception fires, recovers once via `sigsetjmp`/`siglongjmp`, and then finishes execution.
- SIGSEGV handler registers with `signal(SIGSEGV, ...)`, attempts an illegal write, and terminates cleanly.
- Each handler uses async-signal-safe calls (`write`, `_exit`) where required

**Known issues**: None.

---

### 2) Word counter in C (argv filename, case-insensitive, punctuation stripped, top 20, linked list)
**Build**
```bash
gcc c/word_counter/word_counter.c c/shared/linkedlist.c -o word_counter
```

**Run**
```bash
$ ./word_counter data/wctest.txt
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

**Requirement checklist (a–d)**
1. Case-insensitive: tokens are lowercased before counting (`to_lowercase`).
2. Punctuation ignored: tokenizer keeps `isalnum` characters, strips everything else, and warns once if a token exceeds 127 chars.
3. Filename from CLI: program requires exactly one argv entry and shows a usage message otherwise.
4. Top 20 words descending: linked list counts feed a `qsort`ed array before printing.
5. Linked list from Project 4: `c/shared/linkedlist.{c,h}` is the same implementation I submitted in Task 3.

**Other notes**
- Handles empty files by printing "No words found." (documented as the robustness extension below).

**Known issues**: None; prints a warning if a token is truncated, by design.

---

### 3) Memory allocation timing experiment
**Build**
```bash
gcc c/memory_timing/malloc_timing.c -o malloc_timing
```

**Run**
```bash
$ ./malloc_timing
Average allocation time per malloc call:
Scenario	Iterations	Average seconds
100 ints	  50000	0.000000047
10K ints	  10000	0.000000397
1M ints 	    500	0.000000396
```

**Findings**
- Each scenario warms up once, then measures hundreds/thousands of allocations, so the averages above exclude first-call startup cost as requested.
- Trend: per-call cost climbs sharply from 100 ints to 10K ints (allocator metadata + cache effects) and flattens slightly for 1M ints (dominated by page faults). This matches the line chart on the Google Site.
- Allocating many small chunks shows a higher relative overhead than allocating the same total bytes at once because the fixed allocator bookkeeping cost dominates tiny requests.
- The README discussion plus the Google Sites plot together answer the "Is it more efficient ...?" question with data (small vs. large batches) and explain the non-linear behavior.

**Known issues**: None. The numbers vary slightly run-to-run due to OS scheduling; I note that on the report.

---

### 4) Mark-and-sweep garbage collector simulation
**Build**
```bash
gcc c/gc_demo/detect_garbage.c -o detect_garbage
gcc c/gc_sim/mark_sweep.c -o mark_sweep
```

**Run & sample output**
```
$ ./detect_garbage
HeapChunk[0] @ 0x... is reachable.
HeapChunk[1] @ 0x... is reachable.
HeapChunk[2] @ 0x... is reachable.
HeapChunk[3] @ 0x... is reachable.
HeapChunk[4] @ 0x... is garbage.
HeapChunk[5] @ 0x... is garbage.

$ ./mark_sweep
Initial program state:
  Stack roots: rootA -> alpha, rootB -> beta, helper -> NULL
  Heap includes a cycle (cycle1 <-> cycle2) plus reachable chunks
Running mark-and-sweep garbage collector...
Program state after GC:
  Only alpha/beta/gamma/delta remain; the unreachable cycle was freed.
```

ASCII drawing of the scenario described above:

```
Stack roots                    Heap before GC
------------                   ------------------------------
 a -> alpha  ---------------->  alpha --> beta --> gamma --> delta
 b -> beta                     |                    ^
 c -> gamma                    |                    |
 helper -> NULL (later)        |                    +---- back to alpha
                               |
 Unreachable island: cycle_left <----> cycle_right (no stack pointer)

After the mark phase the alpha/beta/gamma/delta nodes remain marked (reachable),
while the two-node cycle stays unmarked and is reclaimed during sweep.
```

**Requirement coverage**
- `detect_garbage.c` creates ≥3 stack vars (`a`, `b`, `c`), ≥4 heap chunks, and an unreachable two-node cycle, then reports which chunks are garbage. That satisfies the "construct your own example" requirement.
- `mark_sweep.c` stores all Vars/HeapChunks inside a `ProgramState`, runs a DFS mark, then sweeps to actually free garbage. The before/after dump demonstrates why reference counting fails for cycles.

**Known issues**: None; both programs exit 0.

---

## Part II — Selected Language: R
R is the selected higher-level language. All scripts below print their own demonstration output.
### 1) Word frequency task (R)
**Build/Run**
```bash
$ Rscript R/word_counter.R data/wctest.txt
```

**Output**
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

**Explanation**
- Uses base R (`readLines`, `tolower`, `gsub`, `strsplit`, `table`) to normalize words, drop punctuation, and produce the top 20 counts in descending order.
- Command-line handling matches the spec: missing filename prints `Usage: Rscript word_counter.R <file>` and exits status 1.
- The script reuses the same test file (`data/wctest.txt`)

**Known issues**: None.

---

### 2) Error handling, file/binary/URL I/O, and interactive input (R)
**Build/Run**
```bash
$ Rscript R/error_and_io_examples.R
```

**Output excerpt**
```
-- Error handling --
Example: tryCatch with specific handlers
  finally after x=2
  result: 5
  caught warning: x is negative
  ...
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

**Answers to the Part II questions**
- **Error handling structures**: `tryCatch` provides typed handlers for errors/warnings (demonstrated with arithmetic cases), while `withCallingHandlers` plus `invokeRestart("muffleWarning")` shows how to intercept warnings without unwinding.
- **How to open/close/read text files**: `file()` creates a connection, `readLines()` reads, and `on.exit(close(con), add = TRUE)` ensures closure. Text demo writes/reads `sample.txt`.
- **Binary file support**: `writeBin` + `readBin` serialize integers in little-endian format; the script prints the round-trip values.
- **Built-in vs. library**: These capabilities live in base R (`utils` package), so no external libraries are required.
- **Opening URLs**: The script opens `https://httpbin.org/uuid` through `url(..., open = "r")`, guarded by `tryCatch` so offline runs report the issue instead of crashing.
- **Interactive input**: `readline()` (guarded by `interactive()`) allows prompting the user, satisfying the "Can the user input information interactively?" question.

**Known issues**: None; network failures simply print "Unable to fetch URL: ..." and the rest of the demo continues.

---

### 3) Memory management research & allocation examples (R)
- R automatically allocates memory for vectors, lists, and environments; explicit `free` is not exposed. Examples: `safe_read_lines` concatenates strings, `word_counter.R` builds `table`s, and `gc_timing.R` creates thousands of numeric vectors via `runif`.
- Modern R (>=3.5) uses a generational, mark-and-sweep collector with lazy copying (copy-on-modify) and an ALTREP layer for compact representations. Stack roots are tracked via the C-side protection stack, and unreachable objects are reclaimed during GC cycles.
- Because allocation is implicit, memory "gets lost" whenever objects fall out of scope (`alloc_burst` returns without storing the list), at which point the GC eventually sweeps them. I describe these mechanics, with citations, on the Google Site.

---

### 4) Garbage-collection timing experiment (R)
**Build/Run**
```bash
$ Rscript R/gc_timing.R 300
```

**Output (trimmed)**
```
Running 300 iterations...
Median time: 0.043500 s
MAD-scaled threshold: 0.059067 s
Detected 87 suspected GC spikes at iterations: 3, 6, 9, ... 298

First 10 iteration timings (s):
  1: 0.044000
  2: 0.045000
  3: 0.078000
  4: 0.043000
  5: 0.043000
  6: 0.069000
  7: 0.043000
  8: 0.042000
  9: 0.060000
 10: 0.041000
```

**Explanation**
- `alloc_burst()` allocates and discards thousands of random vectors per iteration to stress the GC.
- The script records `proc.time()[[3]]` per call, computes the median absolute deviation, and flags spikes (≥ median + 3×MAD) as likely GC pauses, fulfilling the "automatically detect garbage collections" extension.
- Timings and spike indices are exported in the README and plotted on the Google Site.

**Known issues**: None; runtime varies with hardware load.

---

## Extensions

### Extension 1 — Robust C word counter
**Build**
```bash
gcc c/word_counter/word_counter.c c/shared/linkedlist.c -o word_counter_ext
```

**Extra behaviors (beyond the base spec)**
- CLI validation with human-readable usage text when the filename argument is missing or when too many args are passed.
- Clear error message on `fopen` failure, including the OS strerror text.
- Graceful handling of empty files (prints “No words found.” and exits 0).
- Token-length guard that warns when any word exceeds 127 characters instead of overflowing the buffer.

**Example session**
```
$ ./word_counter_ext
Usage: ./word_counter_ext <file>

$ ./word_counter_ext /tmp/nofile
Failed to open '/tmp/nofile': No such file or directory

$ : > /tmp/empty && ./word_counter_ext /tmp/empty
No words found.
```

These additions are isolated in the “Extension: Robustness & Profiling” block near the top of `c/word_counter/word_counter.c`, making it clear where the base requirement ends and the extension begins.

### Extension 2 — Profiling with gprof
**Build**
```bash
gcc -pg c/word_counter/word_counter.c c/shared/linkedlist.c -o word_counter_pg
```

**Run & inspect**
```bash
$ ./word_counter_pg data/wctest.txt   # generates gmon.out
$ gprof ./word_counter_pg gmon.out | head -n 15
%   cumulative   self                self     total
 time   seconds   seconds      calls   s/call   s/call  name
 62.5     0.000     0.000     12500     0.000     0.000  read_next_word
 25.0     0.000     0.000         -         -         -  ll_find
 ...
```

**Notes**
- The profiler confirms that tokenization and linked-list lookups dominate runtime.
- This workflow is separate from the base grading path: only the profiling build uses `-pg`, and its artifacts (`gmon.out`) are intentionally left out of the repo.


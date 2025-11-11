/* detect_garbage.c
 * YOUR NAME HERE
 * Identify garbage heap chunks using the mark-and-sweep method
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_SIZE 10
#define MAX_HEAP_SIZE 10

// A chunk on the heap could refer to other chunks on the heap
// the "marked" field is for use by the mark and sweep algorithm and
// isn't part of the data the code accesses.
typedef struct _HeapChunk {
    int num_references;
    int marked;
    struct _HeapChunk **references;
} HeapChunk;

// Var on stack. Can have value that is a reference to the Heap, or it could
// have a different value.
// We indicate its value is on heap by having reference by non-null
typedef struct {
    char *name;
    HeapChunk *reference;
} Var;

// Program state has stack and heap separately both stored as array for
// simplicity
typedef struct {
    Var *stack;  // array of all current variables
    int num_vars_on_stack;

    // array of all allocated HeapChunks (so array of pointers)
    HeapChunk **heap;
    int num_heap_chunks;
} ProgramState;

// make a new program state
ProgramState *createProgramState() {
    ProgramState *state = (ProgramState *)malloc(sizeof(ProgramState));
    state->stack = (Var *)malloc(sizeof(Var) * MAX_STACK_SIZE);
    state->heap = (HeapChunk **)malloc(sizeof(HeapChunk *) * MAX_HEAP_SIZE);
    state->num_heap_chunks = 0;
    state->num_vars_on_stack = 0;
    return state;
}

static char *duplicateString(const char *input) {
    size_t length = strlen(input) + 1;
    char *copy = (char *)malloc(length);
    if (!copy) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    memcpy(copy, input, length);
    return copy;
}

// allocate a chunk and return the address to it,
// but also add it to our list of allocated chunks on heap
HeapChunk *HeapMalloc(ProgramState *state) {
    HeapChunk *chunk = (HeapChunk *)malloc(sizeof(HeapChunk));
    chunk->num_references = 0;
    chunk->references = NULL;
    state->heap[state->num_heap_chunks] = chunk;
    state->num_heap_chunks++;
    return chunk;
}

// update the stack (either add name/value pair or update value)
void setVar(ProgramState *state, char *var_name, HeapChunk *chunk) {
    int found = 0;
    for (int i = 0; i < state->num_vars_on_stack; i++) {
        if (strcmp(state->stack[i].name, var_name) == 0) {
            found = 1;
            state->stack[i].reference = chunk;
        }
    }
    if (!found) {
        state->stack[state->num_vars_on_stack].name = duplicateString(var_name);
        state->stack[state->num_vars_on_stack].reference = chunk;
        state->num_vars_on_stack++;
    }
}

// adds a reference from chunk_source to chunk_target.
// method is humorously badly written, don't try to do this for too many
// references.
void addReference(HeapChunk *chunk_source, HeapChunk *chunk_target) {
    if (chunk_source->num_references++ == 0)
        chunk_source->references = malloc(sizeof(HeapChunk *));
    else
        chunk_source->references =
            realloc(chunk_source->references,
                       chunk_source->num_references * sizeof(HeapChunk *));
    chunk_source->references[chunk_source->num_references - 1] = chunk_target;
}

// Mark and Sweep, but don't really sweep - just report which chunks are garbage.
// First, you should unmark all the heap chunks by setting the mark value to 0. 
// You need to use the heap chunk array to access all heap chunks
// Then, you should trace through all heap chunks you can find from the stack.
// You need to use the stack and then implement a depth first search that follows
// all references (even from HeapChunk to HeapChunk).  Be sure not to get
// caught in any circular references (i.e. think about your stopping conditions).
// Finally, you should loop through the heap array again, this time reporting for each
// HeapChunk whether it is reachable or garbage.
static void markChunk(HeapChunk *chunk) {
    if (!chunk || chunk->marked) {
        return;
    }

    chunk->marked = 1;

    for (int i = 0; i < chunk->num_references; i++) {
        markChunk(chunk->references[i]);
    }
}

void markAndSweep(ProgramState *state) {
    if (!state) {
        return;
    }

    // Unmark all chunks before performing the depth-first traversal.
    for (int i = 0; i < state->num_heap_chunks; i++) {
        if (state->heap[i]) {
            state->heap[i]->marked = 0;
        }
    }

    // Mark any chunks that can be reached from the stack roots.
    for (int i = 0; i < state->num_vars_on_stack; i++) {
        markChunk(state->stack[i].reference);
    }

    // Report which chunks are still reachable and which are garbage.
    for (int i = 0; i < state->num_heap_chunks; i++) {
        HeapChunk *chunk = state->heap[i];
        const char *status = (chunk && chunk->marked) ? "reachable" : "garbage";
        printf("HeapChunk[%d] @ %p is %s.\n", i, (void *)chunk, status);
    }
}


int main() {
    // Create the initial state
    ProgramState *state = createProgramState();

    //  simulate this simple example.
    // a = malloc()
    // b = malloc()
    // a = b;
//     HeapChunk *aChunk = HeapMalloc(state);
//     setVar(state, "a", aChunk);  // a = malloc()  (Drawing 1)
//     HeapChunk *bChunk = HeapMalloc(state);
//     setVar(state, "b", bChunk);    // b = malloc() (Drawing 2)
//     setVar(state, "a", bChunk);    // a = b (Drawing 3)

    // simulation an example that includes pointers in a cycle
    // a = malloc();
    // b = malloc();
    // a->b = b;
    // c = malloc();
    // b = malloc();
    // c = b;
    HeapChunk *aChunk = HeapMalloc(state);
    setVar(state, "a", aChunk);  // a = malloc().
    HeapChunk *bChunk = HeapMalloc(state);
    setVar(state, "b", bChunk);    // b = malloc()
    addReference(aChunk, bChunk);  // a->ref = b
    addReference(bChunk, aChunk); // b->ref = a
    // the above line will make your GC loop infinitely if you're not careful!
    HeapChunk *cChunk = HeapMalloc(state);
    setVar(state, "c", cChunk);  // c = malloc()
    bChunk = HeapMalloc(state);
    setVar(state, "b", bChunk);  // b = malloc()
    setVar(state, "c", bChunk);
    
    // Identify the garbage    
    markAndSweep(state); // (Drawing 5)
}

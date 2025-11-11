#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LENGTH 32

typedef struct HeapChunk HeapChunk;

typedef struct Var {
    char name[MAX_NAME_LENGTH];
    HeapChunk *ref;
} Var;

struct HeapChunk {
    char label[MAX_NAME_LENGTH];
    int marked;
    size_t reference_capacity;
    HeapChunk **references;
};

typedef struct ProgramState {
    Var *stack;
    size_t stack_size;
    size_t stack_capacity;
    HeapChunk **heap;
    size_t heap_size;
    size_t heap_capacity;
} ProgramState;

static ProgramState create_program_state(size_t stack_capacity, size_t heap_capacity) {
    ProgramState state;
    state.stack = calloc(stack_capacity, sizeof(*state.stack));
    state.heap = calloc(heap_capacity, sizeof(*state.heap));
    if (!state.stack || !state.heap) {
        fprintf(stderr, "Failed to allocate program state.\n");
        exit(EXIT_FAILURE);
    }
    state.stack_size = 0;
    state.stack_capacity = stack_capacity;
    state.heap_size = 0;
    state.heap_capacity = heap_capacity;
    return state;
}

static void destroy_program_state(ProgramState *state) {
    if (!state) {
        return;
    }
    for (size_t i = 0; i < state->heap_size; ++i) {
        free(state->heap[i]->references);
        free(state->heap[i]);
    }
    free(state->heap);
    free(state->stack);
}

static HeapChunk *allocate_chunk(ProgramState *state, const char *label, size_t reference_capacity) {
    if (state->heap_size >= state->heap_capacity) {
        fprintf(stderr, "Heap capacity exceeded when allocating %s.\n", label);
        exit(EXIT_FAILURE);
    }

    HeapChunk *chunk = calloc(1, sizeof(*chunk));
    if (!chunk) {
        fprintf(stderr, "Failed to allocate heap chunk %s.\n", label);
        exit(EXIT_FAILURE);
    }

    strncpy(chunk->label, label, sizeof(chunk->label) - 1);
    chunk->label[sizeof(chunk->label) - 1] = '\0';
    chunk->reference_capacity = reference_capacity;
    chunk->references = calloc(reference_capacity, sizeof(*chunk->references));
    if (!chunk->references && reference_capacity > 0) {
        fprintf(stderr, "Failed to allocate reference array for %s.\n", label);
        free(chunk);
        exit(EXIT_FAILURE);
    }

    state->heap[state->heap_size++] = chunk;
    return chunk;
}

static Var *find_variable(ProgramState *state, const char *name) {
    for (size_t i = 0; i < state->stack_size; ++i) {
        if (strcmp(state->stack[i].name, name) == 0) {
            return &state->stack[i];
        }
    }
    return NULL;
}

static void update_stack(ProgramState *state, const char *name, HeapChunk *chunk) {
    Var *var = find_variable(state, name);
    if (!var) {
        if (state->stack_size >= state->stack_capacity) {
            fprintf(stderr, "Stack capacity exceeded when updating %s.\n", name);
            exit(EXIT_FAILURE);
        }
        var = &state->stack[state->stack_size++];
        strncpy(var->name, name, sizeof(var->name) - 1);
        var->name[sizeof(var->name) - 1] = '\0';
    }
    var->ref = chunk;
}

static void connect_chunks(HeapChunk *from, size_t index, HeapChunk *to) {
    if (index >= from->reference_capacity) {
        fprintf(stderr, "Reference index %zu out of bounds for %s.\n", index, from->label);
        exit(EXIT_FAILURE);
    }
    from->references[index] = to;
}

static void mark_chunk(HeapChunk *chunk) {
    if (!chunk || chunk->marked) {
        return;
    }
    chunk->marked = 1;
    for (size_t i = 0; i < chunk->reference_capacity; ++i) {
        mark_chunk(chunk->references[i]);
    }
}

static void mark_phase(ProgramState *state) {
    for (size_t i = 0; i < state->stack_size; ++i) {
        mark_chunk(state->stack[i].ref);
    }
}

static void sweep_phase(ProgramState *state) {
    size_t write_index = 0;
    for (size_t read_index = 0; read_index < state->heap_size; ++read_index) {
        HeapChunk *chunk = state->heap[read_index];
        if (!chunk->marked) {
            free(chunk->references);
            free(chunk);
            continue;
        }
        chunk->marked = 0;
        state->heap[write_index++] = chunk;
    }
    state->heap_size = write_index;
}

static void run_gc(ProgramState *state) {
    puts("\nRunning mark-and-sweep garbage collector...");
    mark_phase(state);
    sweep_phase(state);
}

static void print_state(const ProgramState *state) {
    puts("\nStack:");
    for (size_t i = 0; i < state->stack_size; ++i) {
        const Var *var = &state->stack[i];
        printf("  %s -> %s\n", var->name, var->ref ? var->ref->label : "NULL");
    }

    puts("\nHeap:");
    for (size_t i = 0; i < state->heap_size; ++i) {
        const HeapChunk *chunk = state->heap[i];
        printf("  %s (marked=%d) refs:", chunk->label, chunk->marked);
        for (size_t r = 0; r < chunk->reference_capacity; ++r) {
            printf(" %s", chunk->references[r] ? chunk->references[r]->label : "NULL");
        }
        puts("");
    }
}

static void build_demo_state(ProgramState *state) {
    HeapChunk *alpha = allocate_chunk(state, "alpha", 2);
    HeapChunk *beta = allocate_chunk(state, "beta", 2);
    HeapChunk *gamma = allocate_chunk(state, "gamma", 2);
    HeapChunk *delta = allocate_chunk(state, "delta", 1);
    HeapChunk *cycle1 = allocate_chunk(state, "cycle1", 1);
    HeapChunk *cycle2 = allocate_chunk(state, "cycle2", 1);

    update_stack(state, "rootA", alpha);
    update_stack(state, "rootB", beta);
    update_stack(state, "helper", gamma);

    connect_chunks(alpha, 0, beta);
    connect_chunks(beta, 0, delta);
    connect_chunks(beta, 1, gamma);
    connect_chunks(gamma, 0, alpha);
    connect_chunks(delta, 0, gamma);

    connect_chunks(cycle1, 0, cycle2);
    connect_chunks(cycle2, 0, cycle1);

    update_stack(state, "helper", NULL);
}

int main(void) {
    ProgramState state = create_program_state(8, 16);

    build_demo_state(&state);

    puts("Initial program state:");
    print_state(&state);

    run_gc(&state);

    puts("\nProgram state after GC:");
    print_state(&state);

    destroy_program_state(&state);
    return EXIT_SUCCESS;
}

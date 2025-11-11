#define _POSIX_C_SOURCE 200809L

/**
 * @file malloc_timing.c
 * @author Max Petite
 * @date 2025-11-11
 *
 * Measures average malloc latency across various allocation sizes.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct AllocationScenario {
    const char *label;
    size_t element_count;
    size_t iterations;
} AllocationScenario;

/* Convert a timespec to fractional seconds. */
static double timespec_to_seconds(const struct timespec *ts) {
    return ts->tv_sec + ts->tv_nsec / 1e9;
}

/* Average the allocation latency for a specific payload size. */
static double measure_malloc_time(size_t element_count, size_t iterations) {
    const size_t bytes = element_count * sizeof(int);

    /* Warm-up allocation to avoid startup cost. */
    int *warmup = malloc(bytes);
    if (!warmup) {
        fprintf(stderr, "Warm-up allocation failed for %zu elements.\n", element_count);
        exit(EXIT_FAILURE);
    }
    memset(warmup, 0, bytes);
    free(warmup);

    struct timespec start = {0, 0};
    struct timespec end = {0, 0};

    double accumulated = 0.0;

    for (size_t i = 0; i < iterations; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        int *buffer = malloc(bytes);
        clock_gettime(CLOCK_MONOTONIC, &end);

        if (!buffer) {
            fprintf(stderr, "Allocation failed at iteration %zu for %zu elements.\n", i, element_count);
            exit(EXIT_FAILURE);
        }

        accumulated += timespec_to_seconds(&end) - timespec_to_seconds(&start);
        memset(buffer, 0, bytes);
        free(buffer);
    }

    return accumulated / iterations;
}

/* Iterate through the scenarios and print the timing table. */
int main(void) {
    const AllocationScenario scenarios[] = {
        {"100 ints", 100, 50000},
        {"10K ints", 10000, 10000},
        {"1M ints", 1000000, 500}
    };

    const size_t scenario_count = sizeof(scenarios) / sizeof(scenarios[0]);

    puts("Average allocation time per malloc call:");
    puts("Scenario\tIterations\tAverage seconds");

    for (size_t i = 0; i < scenario_count; ++i) {
        double average = measure_malloc_time(scenarios[i].element_count, scenarios[i].iterations);
        printf("%-8s\t%7zu\t%.9f\n",
               scenarios[i].label,
               scenarios[i].iterations,
               average);
    }

    return EXIT_SUCCESS;
}

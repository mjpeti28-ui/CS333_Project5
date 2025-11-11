#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../shared/linkedlist.h"

/*
 * Extension: Robustness & Profiling (explicitly marked)
 * - Robust CLI/file handling (argc check, fopen error): already present.
 * - Empty-file handling: if no tokens are found, print a friendly message and exit.
 * - Token-length warning: warn once if any token exceeds MAX_WORD_LENGTH-1 and is truncated.
 * - Profiling note: build with `gcc -pg c/word_counter/word_counter.c linkedlist.c -o word_counter_pg`
 *   and run `gprof` as shown in README.
 */

#define MAX_WORD_LENGTH 128

typedef struct WordCount {
    char *word;
    size_t count;
} WordCount;

static void to_lowercase(char *word) {
    for (char *p = word; *p; ++p) {
        *p = (char)tolower((unsigned char)*p);
    }
}

static char *duplicate_word(const char *word) {
    size_t length = strlen(word) + 1;
    char *copy = (char *)malloc(length);
    if (!copy) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    memcpy(copy, word, length);
    return copy;
}

static WordCount *create_wordcount(const char *word) {
    WordCount *wc = (WordCount *)malloc(sizeof(*wc));
    if (!wc) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    wc->word = duplicate_word(word);
    wc->count = 1;
    return wc;
}

/* Extension: track whether we truncated any token due to MAX_WORD_LENGTH. */
static int g_truncated_token_seen = 0;

static int comp_word_to_key(void *payload, void *target) {
    const WordCount *wc = (const WordCount *)payload;
    const char *key = (const char *)target;
    return wc && wc->word && key && strcmp(wc->word, key) == 0;
}

static void free_wordcount(void *payload) {
    if (!payload) return;
    WordCount *wc = (WordCount *)payload;
    free(wc->word);
    free(wc);
}

static void add_or_increment(LinkedList *list, const char *word) {
    WordCount *found = (WordCount *)ll_find(list, (void *)word, comp_word_to_key);
    if (found) {
        found->count++;
        return;
    }
    WordCount *wc = create_wordcount(word);
    ll_append(list, wc);
}

static int read_next_word(FILE *file, char *buffer, size_t buffer_size) {
    int ch;
    size_t index = 0;

    while ((ch = fgetc(file)) != EOF) {
        if (isalnum(ch) || ch == '\'') {
            if (index + 1 < buffer_size) {
                buffer[index++] = (char)ch;
            } else {
                /* Extension: record that we hit the max token length. */
                g_truncated_token_seen = 1;
                break;
            }
        } else if (index > 0) {
            break;
        }
    }

    if (index == 0) {
        return 0;
    }

    buffer[index] = '\0';

    size_t write_index = 0;
    for (size_t i = 0; buffer[i] != '\0'; ++i) {
        if (isalnum((unsigned char)buffer[i])) {
            buffer[write_index++] = buffer[i];
        }
    }
    buffer[write_index] = '\0';

    if (write_index == 0) {
        return 0;
    }

    to_lowercase(buffer);
    return 1;
}

static int cmp_wordcount_desc(const void *a, const void *b) {
    const WordCount *wa = *(const WordCount *const *)a;
    const WordCount *wb = *(const WordCount *const *)b;
    if (wa->count != wb->count) {
        return (wb->count > wa->count) - (wb->count < wa->count);
    }
    return strcmp(wa->word, wb->word);
}

static WordCount **list_to_array(LinkedList *list, size_t *out_size) {
    int n = ll_size(list);
    if (n < 0) n = 0;
    WordCount **array = (WordCount **)malloc((size_t)n * sizeof(*array));
    if (!array) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    size_t i = 0;
    for (Node *cur = list->head; cur != NULL; cur = cur->next) {
        array[i++] = (WordCount *)cur->data;
    }
    *out_size = (size_t)n;
    return array;
}

static void print_top_words(LinkedList *list, size_t limit) {
    size_t size = 0;
    WordCount **array = list_to_array(list, &size);
    qsort(array, size, sizeof(*array), cmp_wordcount_desc);

    size_t to_print = size < limit ? size : limit;
    for (size_t i = 0; i < to_print; ++i) {
        printf("%-10s\t%zu\n", array[i]->word, array[i]->count);
    }

    free(array);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open '%s': %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }

    LinkedList *list = ll_create();
    if (!list) {
        fprintf(stderr, "Failed to create list.\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    char word_buffer[MAX_WORD_LENGTH];
    while (read_next_word(file, word_buffer, sizeof(word_buffer))) {
        add_or_increment(list, word_buffer);
    }

    fclose(file);

    /* Extension: handle empty file (or no tokens) explicitly. */
    if (ll_size(list) == 0) {
        puts("No words found.");
        ll_clear(list, free_wordcount);
        free(list);
        return EXIT_SUCCESS;
    }

    puts("Top 20 words by frequency:");
    print_top_words(list, 20);

    /* Extension: warn once if any token was truncated. */
    if (g_truncated_token_seen) {
        fprintf(stderr, "Warning: one or more tokens exceeded %d characters and were truncated.\n",
                MAX_WORD_LENGTH - 1);
    }

    ll_clear(list, free_wordcount);
    free(list);
    return EXIT_SUCCESS;
}

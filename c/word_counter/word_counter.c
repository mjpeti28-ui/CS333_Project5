#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH 128

typedef struct WordNode {
    char *word;
    size_t count;
    struct WordNode *next;
} WordNode;

static void to_lowercase(char *word) {
    for (char *p = word; *p; ++p) {
        *p = (char)tolower((unsigned char)*p);
    }
}

static char *duplicate_word(const char *word) {
    size_t length = strlen(word) + 1;
    char *copy = malloc(length);
    if (!copy) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    memcpy(copy, word, length);
    return copy;
}

static WordNode *create_node(const char *word) {
    WordNode *node = (WordNode *)malloc(sizeof(*node));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    node->word = duplicate_word(word);
    node->count = 1;
    node->next = NULL;
    return node;
}

static void add_word(WordNode **head, const char *word) {
    if (!*head) {
        *head = create_node(word);
        return;
    }

    WordNode *current = *head;
    WordNode *previous = NULL;

    while (current) {
        if (strcmp(current->word, word) == 0) {
            current->count++;
            return;
        }
        previous = current;
        current = current->next;
    }

    WordNode *node = create_node(word);
    previous->next = node;
}

static void free_list(WordNode *head) {
    while (head) {
        WordNode *next = head->next;
        free(head->word);
        free(head);
        head = next;
    }
}

static size_t list_length(const WordNode *head) {
    size_t length = 0;
    while (head) {
        ++length;
        head = head->next;
    }
    return length;
}

static WordNode **list_to_array(WordNode *head, size_t *out_size) {
    size_t length = list_length(head);
    WordNode **array = malloc(length * sizeof(*array));
    if (!array) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    size_t index = 0;
    while (head) {
        array[index++] = head;
        head = head->next;
    }

    *out_size = length;
    return array;
}

static int compare_nodes(const void *lhs, const void *rhs) {
    const WordNode *const *a = lhs;
    const WordNode *const *b = rhs;

    if ((*a)->count != (*b)->count) {
        return ((*b)->count > (*a)->count) - ((*b)->count < (*a)->count);
    }
    return strcmp((*a)->word, (*b)->word);
}

static int read_next_word(FILE *file, char *buffer, size_t buffer_size) {
    int ch;
    size_t index = 0;

    while ((ch = fgetc(file)) != EOF) {
        if (isalnum(ch) || ch == '\'') {
            buffer[index++] = (char)ch;
            if (index + 1 >= buffer_size) {
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

static void print_top_words(WordNode *head, size_t limit) {
    size_t size = 0;
    WordNode **array = list_to_array(head, &size);

    qsort(array, size, sizeof(*array), compare_nodes);

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

    WordNode *head = NULL;
    char word_buffer[MAX_WORD_LENGTH];

    while (read_next_word(file, word_buffer, sizeof(word_buffer))) {
        add_word(&head, word_buffer);
    }

    fclose(file);

    puts("Top 20 words by frequency:");
    print_top_words(head, 20);

    free_list(head);
    return EXIT_SUCCESS;
}

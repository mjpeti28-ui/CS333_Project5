/**
 * @file linkedlist.h
 * @author Max Petite
 * @date 2025-11-11
 *
 * Declares a generic singly linked list interface.
 */

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stddef.h>

typedef struct Node {
    void *data;
    struct Node *next;
} Node;

typedef struct LinkedList {
    Node *head;
} LinkedList;

LinkedList *ll_create(void);
/* Prepends a node to the list. */
void ll_push(LinkedList *list, void *data);
/* Removes and returns the head node's payload. */
void *ll_pop(LinkedList *list);
/* Appends a node to the tail of the list. */
void ll_append(LinkedList *list, void *data);
/* Removes the first node whose payload matches target. */
void *ll_remove(LinkedList *list, void *target, int (*compfunc)(void *, void *));
/* Returns the first payload matching target without removing it. */
void *ll_find(LinkedList *list, void *target, int (*compfunc)(void *, void *));
int ll_size(LinkedList *list);
/* Clears the list, freeing payloads via freefunc when provided. */
void ll_clear(LinkedList *list, void (*freefunc)(void *));
/* Applies a function to each payload in order. */
void ll_map(LinkedList *list, void (*mapfunc)(void *));

/* Extension: removes and returns the payload at zero-based index. */
void *ll_delete_at(LinkedList *list, int index);

#endif /* LINKEDLIST_H */

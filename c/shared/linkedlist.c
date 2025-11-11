#include <stdlib.h>

#include "linkedlist.h"

/**
 * @file linkedlist.c
 * @author Max Petite
 * @date 2025-10-30
 *
 * Implements the generic singly linked list API.
 */

/* Allocates and initialises an empty list handle. */
LinkedList *ll_create(void) {
    LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
    if (list != NULL) {
        list->head = NULL;
    }
    return list;
}

/* Inserts data at the head of the list. */
void ll_push(LinkedList *list, void *data) {
    if (list == NULL) {
        return;
    }

    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        return;
    }

    node->data = data;
    node->next = list->head;
    list->head = node;
}

/* Removes the head node and returns its payload. */
void *ll_pop(LinkedList *list) {
    if (list == NULL || list->head == NULL) {
        return NULL;
    }

    Node *head = list->head;
    void *data = head->data;

    list->head = head->next;
    free(head);

    return data;
}

/* Appends data at the tail of the list. */
void ll_append(LinkedList *list, void *data) {
    if (list == NULL) {
        return;
    }

    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        return;
    }

    node->data = data;
    node->next = NULL;

    if (list->head == NULL) {
        list->head = node;
        return;
    }

    Node *current = list->head;
    while (current->next != NULL) {
        current = current->next;
    }

    current->next = node;
}

/* Removes the first node that satisfies compfunc against target. */
void *ll_remove(LinkedList *list, void *target, int (*compfunc)(void *, void *)) {
    if (list == NULL || compfunc == NULL) {
        return NULL;
    }

    Node **link = &list->head;
    while (*link != NULL) {
        if (compfunc((*link)->data, target)) {
            Node *match = *link;
            void *data = match->data;
            *link = match->next;
            free(match);
            return data;
        }
        link = &(*link)->next;
    }

    return NULL;
}

/* Returns the first payload that matches target without removal. */
void *ll_find(LinkedList *list, void *target, int (*compfunc)(void *, void *)) {
    if (list == NULL || compfunc == NULL) {
        return NULL;
    }

    Node *current = list->head;
    while (current != NULL) {
        if (compfunc(current->data, target)) {
            return current->data;
        }
        current = current->next;
    }

    return NULL;
}

/* Counts how many nodes are present. */
int ll_size(LinkedList *list) {
    if (list == NULL) {
        return 0;
    }

    int count = 0;
    Node *current = list->head;
    while (current != NULL) {
        ++count;
        current = current->next;
    }

    return count;
}

/* Deletes all nodes, invoking freefunc for payload disposal. */
void ll_clear(LinkedList *list, void (*freefunc)(void *)) {
    if (list == NULL) {
        return;
    }

    Node *current = list->head;
    while (current != NULL) {
        Node *next = current->next;
        if (freefunc != NULL) {
            freefunc(current->data);
        }
        free(current);
        current = next;
    }

    list->head = NULL;
}

/* Invokes mapfunc across each payload in order. */
void ll_map(LinkedList *list, void (*mapfunc)(void *)) {
    if (list == NULL || mapfunc == NULL) {
        return;
    }

    Node *current = list->head;
    while (current != NULL) {
        mapfunc(current->data);
        current = current->next;
    }
}

/* Extension: deletes node at the given zero-based index and returns its data. */
void *ll_delete_at(LinkedList *list, int index) {
    if (list == NULL || index < 0) {
        return NULL;
    }

    Node **link = &list->head;
    while (*link != NULL && index > 0) {
        link = &(*link)->next;
        --index;
    }

    if (*link == NULL) {
        return NULL;
    }

    Node *target = *link;
    void *payload = target->data;
    *link = target->next;
    free(target);
    return payload;
}

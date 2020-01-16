#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "globals.h"

/*
 * This module contains global functions that are used over the project, like copying a Board object.
 */

/* copies src into dest, dest should be empty beforehand */
void copyBoard(Board *src, Board *dest) {
    int i, j;
    size_t size;
    size = (size_t) src->size;

    dest->size = src->size;
    dest->m = src->m;
    dest->n = src->n;

    dest->values = (int **) calloc(size, sizeof(int *));
    if (dest->values == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    dest->fixedPositions = (int **) calloc(size, sizeof(int *));
    if (dest->fixedPositions == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    dest->errors = (int **) calloc(size, sizeof(int *));
    if (dest->errors == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    for (i = 0; i < dest->size; i++) {
        dest->values[i] = (int *) calloc(size, sizeof(int));
        if (dest->values[i] == NULL) {
            printf("ERROR: Could not allocate memory.");
            abort();
        }
        dest->fixedPositions[i] = (int *) calloc(size, sizeof(int));
        if (dest->fixedPositions[i] == NULL) {
            printf("ERROR: Could not allocate memory.");
            abort();
        }
        dest->errors[i] = (int *) calloc(size, sizeof(int));
        if (dest->errors[i] == NULL) {
            printf("ERROR: Could not allocate memory.");
            abort();
        }
        for (j = 0; j < dest->size; j++) {
            dest->values[i][j] = src->values[i][j];
            dest->fixedPositions[i][j] = src->fixedPositions[i][j];
            dest->errors[i][j] = src->errors[i][j];
        }
    }
}

void freeBoard(Board *board) {
    int i;

    for (i = 0; i < board->size; i++) {
        free(board->values[i]);
        free(board->fixedPositions[i]);
        free(board->errors[i]);
    }
    free(board->values);
    free(board->fixedPositions);
    free(board->errors);
    free(board);
}

/* Returns a random number between min and max */
int getRandomNumber(int min, int max) {
    int num = (rand() % (max - min + 1)) + min;
    return num;
}

/* Adds item to be the last node in the list pointed to by 'list' */
void addLast(Node *list, Node *item) {
    while (list->next != NULL) {
        list = list->next;
    }
    list->next = item;
    item->prev = list;
}

void freeStringArray(StringArray *array) {
    int i;
    for (i = 0; i < array->size; i++) {
        free(array->array[i]);
    }
    free(array->array);
    free(array);
}

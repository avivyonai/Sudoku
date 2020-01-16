#ifndef FINAL_PROJECT_GLOBALS_H
#define FINAL_PROJECT_GLOBALS_H

typedef struct IntArray {
    int *array;
    int size;
} IntArray;

typedef struct StringArray {
    char **array;
    int size;
} StringArray;

static const int MAX_ARRAY_LENGTH = 1024;

typedef struct Board {
    int **values;
    int **fixedPositions;
    int **errors;
    int size, n, m;
} Board;

typedef enum NodeType {
    N_DUMMY,
    N_SET,
    N_AUTOFILL_GENERATE
} NodeType;

typedef struct Node {
    void *move;
    NodeType type;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct SetMove {
    int x;
    int y;
    int old;
    int new;
}   SetMove;

typedef struct AutofillGenerateMove {
    Node *first;
    int count;
} AutofillGenerateMove;

/* copies src into dest, dest should be empty beforehand */
void copyBoard(Board *src, Board *dest);

/* Returns a random number between min and max */
int getRandomNumber(int, int);

/* Adds item to be the last node in the list pointed to by 'list' */
void addLast(Node *, Node *);

void freeBoard(Board *);

void freeStringArray(StringArray *);

#endif /*FINAL_PROJECT_GLOBALS_H */

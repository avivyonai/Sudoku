#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "main_aux.h"
#include "game.h" 

/*
 * Contains helper functions for the main module, like printing the board.
 */

int isDigitsOnly(char *);
void printLineSeparator(int);
char * strdup(const char *str1);

/* Loads a board saved in a file in the correct format */
int loadBoard(char *path, Board *board, int isSolve) {
    char line[1024], *value, *tmp, *a;
    int m, n, i, j, size;
    FILE *f;

    f = fopen(path, "r");
    if (f == NULL) {
        return 0;
    }

    a =fgets(line, sizeof(line), f);
    value = strtok(line, " ");
    m = atoi(value);
    value = strtok(NULL, " ");
    n = atoi(value);
    size = m * n;

    *board = createBoard(m,n);

    for (i = 0; i < size; i++) {
        a = fgets(line, sizeof(line), f);
        tmp = strdup(line);
        for (j = 0; j < size; j++) {
            value = strtok(tmp, " \n");
            board->values[i][j] = atoi(value);
            if (strchr(value, '.') && isSolve) {
                board->fixedPositions[i][j] = 1;
            }
            tmp = NULL;
        }
    }
    fclose(f);
    return 1;
}

/* Saves a given board to a file in a specified path */
int saveBoard(Board board, char *path) {
    int i, j;
    /* char *value; */
    FILE *f;

    f = fopen(path, "w");
    if (f == NULL) {
        return 0;
    }
    fprintf(f, "%d %d\n", board.m, board.n);
    for (i = 0; i < board.size; i++) {
        for (j = 0; j < board.size; j++) {
            if (board.fixedPositions[i][j]) {
                fprintf(f, "%d.", board.values[i][j]);
            } else {
                fprintf(f, "%d", board.values[i][j]);
            }
            if (j < board.size - 1) {
                fprintf(f, " ");
            }
        }
        if (i < board.size - 1) {
            fprintf(f, "\n");
        }
    }
    fclose(f);
    return 1;
}


/* Returns 1 if a string contains only digits and 0 otherwise */
int isDigitsOnly(char *s) {
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }
    return 1;
}

/* Prints the board to console in the correct format */
void printBoard(Board board, int isMarkErrors, int isSolveMode) {
    int i, j;
    char sign;
    /* fixed, error, */

    for (i = 0; i < board.size; i++) {
        if (i % board.m == 0) {
            printLineSeparator(4*board.size + board.m + 1);
        }
        for (j = 0; j < board.size; j++) {
            if (j % board.n == 0) {
                printf("|");
            }
            sign = (char) (board.fixedPositions[i][j] && isSolveMode ? '.' : (isMarkErrors && board.errors[i][j] ? '*' : ' '));
            printf(" %2c%c", board.values[i][j] == 0 ? ' ' : board.values[i][j] + '0', sign);
        }
        printf("|\n");
    }
    printLineSeparator(4*board.size + board.m + 1);
}

/* Prints line seperator to console */
void printLineSeparator(int size) {
    int i;
    for (i = 0; i < size; i++) {
        printf("-");
    }
    printf("\n");
}

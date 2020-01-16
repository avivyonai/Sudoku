#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "globals.h"
#include "main_aux.h"
#include "backtrack.h"
#include "ilp_solver.h"

/*
 * Handles the backtracking algorithm, that counts the possible solutions of a given board.
 * In this module there is also a function to validate to board and check if it has a solution using ilp_solver.
 */

struct IntArray getPotentialValues(Board board, int x, int y);

static int SOLS = 0;

int backtrack_solve(Board *board, int x, int y);

/* Returns the number of possible solutions for a given board */
int numSolutions(Board *board) {
    Board *copy = (Board *) malloc(sizeof(Board));
    if (copy == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    copyBoard(board, copy);
    SOLS = 0;
    backtrack(copy, 0, 0);
    /* free copy from memory */
    return SOLS;
}

/* return 1 if board is valid */
int validate(Board *board) {
    if (solve_ilp(board,0) == 0){
        return 1;
    }
    return 0;
}

/* returns no. of sols*/
int backtrack(Board *board, int x, int y) {
    IntArray vals;
    int i, val;

    if (x > board->size || y > board->size) {
        SOLS++;
    }

    if (board->values[x][y] > 0) {
        if (y + 1 < board->size) {
            return backtrack(board, x, y + 1);
        } else if (x + 1 < board->size) {
            return backtrack(board, x + 1, 0);
        } else {
            SOLS++;
        }
    } else {
        vals = getPotentialValues(*board, x, y);
        for (i = 0; i < vals.size; i++) {
            val = vals.array[i];
            board->values[x][y] = val;
            if (y + 1 < board->size) {
                backtrack(board, x, y + 1);
            } else if (x + 1 < board->size) {
                backtrack(board, x + 1, 0);
            } else {
                SOLS++;
                board->values[x][y] = 0;
            }
        }

        free(vals.array);
    }
    board->values[x][y] = 0;
    return SOLS;
}

/* checks if given value is valid for a specific cell */
int isValid(Board board, int x, int y, int val) {
    int blockRow, blockCol, i;

    blockRow = (x / board.m) * board.m;
    blockCol = (y / board.n) * board.n;

    for (i = 0; i < board.size; i++) {
        if (board.values[x][i] == val || board.values[i][y] == val ||
            board.values[blockRow + (i % board.m)][blockCol + (i / board.n)] == val) {
            return 0;
        }
    }
    return 1;
}

/* returns an array of possible values for a specific cell */
struct IntArray getPotentialValues(Board board, int x, int y) {
    int val, max;
    struct IntArray values;
    values.size = 0;
    values.array = (int *) malloc(values.size * sizeof(int));
    if (values.array == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }

    max = board.size;

    for (val = 1; val <= max; val++) {
        if (isValid(board, x, y, val)) {
            values.size++;
            values.array = (int *) realloc(values.array, values.size * sizeof(int));
            values.array[values.size - 1] = val;
        }
    }

    return values;
}


#include <stdlib.h>
#include <stdio.h>

#include "globals.h"
#include "game.h"
#include "backtrack.h"
#include "gurobi_c.h"
#include "ilp_solver.h"

#include "main_aux.h"

/*
 * This module contains functions that control the flow of the game, e.g initializing it or  creating a board.
 * There are also helper functions for those methods.
 */

static Board board;
static Node *lastMove = NULL;

/* Initializes settings for a new game */
int initGame() {
    lastMove = (Node *) malloc(sizeof(Node));
    if (lastMove == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    lastMove->type = N_DUMMY;
    lastMove->next = NULL;
    lastMove->prev = NULL;
    return 0; /* added */
}

/* Set a board as the active game board */
void setBoard(Board b) {
    copyBoard(&b, &board);
}

/* Generates a board by randomly filling X cells, then using ILP to solve the board, and keeping Y random cells */
int generateBoard(int X, int Y) {
    Board newBoard;
    AutofillGenerateMove *move;
    int i, j, x, y, rnd, counter = 0, stop = 0, tries = 0;

    while (!stop && tries < 1000) {
        stop = 1;
        counter = 0;
//        newBoard = createBoard(board.n, board.m);
        copyBoard(&board, &newBoard);

        while (counter < X) {
            rnd = getRandomNumber(0, board.size * board.size - 1);
            x = rnd / board.size;
            y = rnd % board.size;
            if (newBoard.values[x][y] == 0) {
                rnd = getRandomLegalValue(newBoard, x, y);
                if (!rnd) {
                    stop = 0;
                    break;
                }
                newBoard.values[x][y] = rnd;
                counter++;

            }
        }
        if (stop) {
            if (solve_ilp(&newBoard,1)) {
                stop = 0;
            }
        }
        tries++;
    }

    if (tries < 1000) {
        counter = 0;
        while (counter < (newBoard.size * newBoard.size) - Y) {
            rnd = getRandomNumber(0, board.size * board.size - 1);
            x = rnd / board.size;
            y = rnd % board.size;
            if (newBoard.values[x][y] > 0) {
                newBoard.values[x][y] = 0;
                counter++;
            }
        }

        move = (AutofillGenerateMove *) malloc(sizeof(AutofillGenerateMove));
        if (move == NULL) {
            printf("ERROR: Could not allocate memory.");
            abort();
        }
        move->first = (Node *) malloc(sizeof(Node));
        if (move->first == NULL) {
            printf("ERROR: Could not allocate memory.");
            abort();
        }
        move->first->next = NULL;
        move->first->prev = NULL;
        move->first->type = N_DUMMY;
        for (i = 0; i < newBoard.size; i++) {
            for (j = 0; j < newBoard.size; j++) {
                if (newBoard.values[i][j] != board.values[i][j]) {
                    Node *action = (Node *) malloc(sizeof(Node));
                    if (action == NULL) {
                        printf("ERROR: Could not allocate memory.");
                        abort();
                    }
                    SetMove *set = (SetMove *) malloc(sizeof(SetMove));
                    if (set == NULL) {
                        printf("ERROR: Could not allocate memory.");
                        abort();
                    }
                    set->x = i;
                    set->y = j;
                    set->old = board.values[i][j];
                    set->new = newBoard.values[i][j];
                    action->move = set;
                    action->next = NULL;
                    action->prev = NULL;

                    addLast(move->first, action);
                }
            }
        }

        addMoveToList(N_AUTOFILL_GENERATE, move);

        board = newBoard;
        return 1;
    }

    return 0;
}

/* Returns a random legal value for a specific cell and board */
int getRandomLegalValue(Board board, int x, int y) {
    int val, rnd;
    IntArray *values = (IntArray *) malloc(sizeof(IntArray));
    if (values == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    values->size = 0;
    values->array = (int *) malloc(values->size * sizeof(int));
    if (values->array == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }

    for (val = 1; val <= board.size; val++) {
        if (isCellValid(board, x, y, val)) {
            values->size++;
            values->array = (int*) realloc(values->array, values->size * sizeof(int));
            if (values->array == NULL) {
                printf("ERROR: Could not allocate memory.");
                abort();
            }
            values->array[values->size - 1] = val;
        }
    }

    if (values->size == 0) {
        return 0;
    }

    rnd = getRandomNumber(0, values->size - 1);
    val = values->array[rnd];
    free(values->array);
    free(values);
    return val;
}

/* Creates a Board struct and returns it */
Board createBoard(int m, int n) {
    Board *board;
    int i;
    int size;

    size = n * m;
    board = (Board *) malloc(sizeof(Board));
    if (board == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }

    board->values = (int **) calloc(size, sizeof(int *));
    if (board->values == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    board->fixedPositions = (int **) calloc(size, sizeof(int *));
    if (board->fixedPositions == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    board->errors = (int **) calloc(size, sizeof(int *));
    if (board->errors == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    for (i = 0; i < size; i++) {
        board->values[i] = (int *) calloc(size, sizeof(int));
        if (board->values[i] == NULL) {
            printf("ERROR: Could not allocate memory.");
            abort();
        }
        board->fixedPositions[i] = (int *) calloc(size, sizeof(int));
        if (board->fixedPositions[i] == NULL) {
            printf("ERROR: Could not allocate memory.");
            abort();
        }
        board->errors[i] = (int *) calloc(size, sizeof(int));
        if (board->errors[i] == NULL) {
            printf("ERROR: Could not allocate memory.");
            abort();
        }
    }

    board->n = n;
    board->m = m;
    board->size = size;

    return *board;
}

/* Returns active board */
Board getBoard() {
    return board;
}

/* Returns a copy of the active board */
void getBoardCopy(Board *copy) {
    copyBoard(&board, copy);
}

/* Returns the number of empty cells in the active board */
int countEmptyCells() {
    int i, j, count = 0;
    for (i = 0; i < board.size; i++) {
        for (j = 0; j < board.size; j++) {
            if (board.values[i][j] == 0)
                count++;
        }
    }
    return count;
}

/* Returns 1 if the board is empty (all values are 0) and 0 otherwise */
int isBoardEmpty() {
    int i, j;
    for (i = 0; i < board.size; i++) {
        for (j = 0; j < board.size; j++) {
            if (board.values[i][j] > 0)
                return 0;
        }
    }
    return 1;
}

/* Return 1 if the board is full (no value equals to 0) and 0 otherwise */
int isBoardFull() {
    int i, j;
    for (i = 0; i < board.size; i++) {
        for (j = 0; j < board.size; j++) {
            if (board.values[i][j] == 0)
                return 0;
        }
    }
    return 1;
}

/* Returns 1 if all values in the board are valid and 0 otherwise */
int isBoardValid() {
    int i, j;
    for (i = 0; i < board.size; i++) {
        for (j = 0; j < board.size; j++) {
            if (board.errors[i][j] > 0)
                return 0;
        }
    }
    return 1;
}

/* checks if given value is valid for a specific cell */
int isCellValid(Board board, int x, int y, int val) {
    int blockRow, blockCol, i,j;

    blockRow = (x / board.m);// * board.n;
    blockCol = (y / board.n); //* board.m;

    for (i = 0; i < board.size; i++) {
        for (j = 0; j < board.size; j++) {
            if ((i != y && board.values[x][i] == val) ||
                (i != x && board.values[i][y] == val) ||
                ((blockRow * board.m + (j % board.m) != x || blockCol * board.n + (i % board.n) != y) &&
                 board.values[(blockRow * board.m) + (j % board.m)][(blockCol * board.n) + (i % board.n)] == val)) {
                return 0;
            }
        }
    }
    return 1;
}

/* Returns the number of possible legal values for a specific cell and board */
int countLegalValues(Board board, int x, int y) {
    int i, count = 0;
    for (i = 1; i <= board.size; i++) {
        if (isCellValid(board, x, y, i)) {
            count++;
        }
    }
    return count;
}

/* Updates the errors marking for the active board - 1 if error exists for cell and 0 otherwise */
void markErrors() {
    int i, j;
    for (i = 0; i < board.size; i++) {
        for (j = 0; j < board.size; j++) {
            if (board.values[i][j] == 0) {
                board.errors[i][j] = 0;
            } else {
                board.errors[i][j] = !isCellValid(board, i, j, board.values[i][j]);
            }
        }
    }
}

/* Add a user move the the moves list */
void addMoveToList(NodeType type, void *move) {
    Node *node = (Node *) malloc(sizeof(Node));
    if (node == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    node->next = NULL;
    node->type = type;
    node->move = move;

    freeNextMoves(lastMove);

    lastMove->next = node;
    node->prev = lastMove;
    lastMove = node;
}

/* Sets a value for a cell in the active board */
int setCell(int y, int x, int val) {
    SetMove *new_move;
    x = x -1;
    y = y - 1;
    if ((y >= board.size || y < 0) || (x >= board.size || x < 0) || (val > board.size || val < 0)) {
        return -1;
    }
    if (board.fixedPositions[x][y] > 0) {
        return 0;
    }
    if (board.values[x][y] == val) {
        return 1;
    }

    new_move = (SetMove *) malloc(sizeof(SetMove));
    if (new_move == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    new_move->x = x;
    new_move->y = y;
    new_move->old = board.values[x][y];
    new_move->new = val;

    board.values[x][y] = val;
    markErrors();

    addMoveToList(N_SET, new_move);

    return 1;
}

/* Returns a possible value that will lead to a solution of the board using ILP */
int hint(int y, int x) {
    Board *newBoard = (Board *) malloc(sizeof(Board));
    getBoardCopy(newBoard);
    if (board.fixedPositions[x][y] != 0) {
        return -1;
    }
    else if (board.values[x][y] != 0){
        return -2;
    }
    solve_ilp(newBoard,1);
    return newBoard->values[x][y];
}

/* running LP to find solution for a specific cell, returns 1 if found, 0 if not, -1 if the cell is fixed and -2 if cell contains a value */
int guess_hint(int y,  int x){
    Board *newBoard = (Board *) malloc(sizeof(Board));
    int legal, result, score,i;
    float* legal_options;
    const int N = board.size;
    getBoardCopy(newBoard);
    if (board.fixedPositions[x][y] != 0) {
        return -1;
    }
    else if (board.values[x][y] != 0){
        return -2;
    }
    /* run LP (copy board) */
    legal_options = (float*)calloc(N*2,sizeof(float)); /* 0->optional values, 1->score */
    legal = LPSolveCell(x+1, y+1, newBoard, legal_options);
    if(!legal){
        freeBoard(newBoard);
        return 0;
    }
    for(i = 0; i<N; i++){
        result = i*2;
        score = i*2+1;
        if(legal_options[score]>=0.0){
            printf("Hint: optional value of <%d,%d> is %d with score %f \n", y+1,x+1,(int)legal_options[result], legal_options[score]);
        }
    }    free(legal_options);
    freeBoard(newBoard);
    return 1;
    /* print the value of specific location in board */
    /* free copied board */

}

/* running LP to find solutions for the whole board, return 1 if LP succeeded, 0 otherwise */
int guess(float x){
    /*function description: fills all cell values with a score of X or greater using LP. If several
   * values hold for the same cell, randomly choose one according to the score
     * state: Solve
     * args: x --> lower boundary of score
     * return: 0 --> there is no solution to the board / an error occurred, 1 --> board has a solution
     */
//    Board newBoard = createBoard(board.m,board.n);
    Board *newBoard = (Board *) malloc(sizeof(Board));
    getBoardCopy(newBoard);
//    newBoard.n = board.n;
//    newBoard.m = board.m;
    int legal;
    /* call LP_Solver: get board and solve it using LP If several
    values hold for the same cell, randomly choose one according to the score */
    legal = LPSolver(x, newBoard);
    freeBoard(newBoard);
    if(!legal){
        printf("ERROR: LP Failed \n");
        return 0;
    }
    return 1;
}

/* Fills all cells that have only one legal value (at the time the function was called) */
Node * autofill() {
    Board *copy = (Board *) malloc(sizeof(Board));
    Node *action;
    SetMove *set;
    int tmp, i=0, j=0;
    AutofillGenerateMove *move = (AutofillGenerateMove *) malloc(sizeof(AutofillGenerateMove));
    if (move == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }

    getBoardCopy(copy);

    move->first = (Node *) malloc(sizeof(Node));
    if (move->first == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    move->first->next = NULL;
    move->first->prev = NULL;
    move->first->type = N_DUMMY;

    for (i = 0; i < copy->size; i++) {
        for (j = 0; j < copy->size; j++) {
            if (copy->values[i][j] == 0 && countLegalValues(*copy, i, j) == 1) {
                tmp = getRandomLegalValue(*copy, i, j);

                action = (Node *) malloc(sizeof(Node));
                if (action == NULL) {
                    printf("ERROR: Could not allocate memory.");
                    abort();
                }
                set = (SetMove *) malloc(sizeof(SetMove));
                if (set == NULL) {
                    printf("ERROR: Could not allocate memory.");
                    abort();
                }
                set->x = i;
                set->y = j;
                set->old = board.values[i][j];
                set->new = tmp;
                action->move = set;
                action->next = NULL;
                action->prev = NULL;

                addLast(move->first, action);

                board.values[i][j] = tmp;

            }
        }
    }

    for (i = 0; i < board.size; i++) {
        for (j = 0; j < board.size; j++) {
            if (board.values[i][j] > 0 && !isCellValid(board, i, j, board.values[i][j])) {
                board.errors[i][j] = 1;
            } else {
                board.errors[i][j] = 0;
            }
        }
    }

    if (move->first->next != NULL)
        addMoveToList(N_AUTOFILL_GENERATE, move);

    freeBoard(copy);

    return move->first;
}

/* Clears all moves */
void resetMoveList() {
    freeNextMoves(lastMove);
    freePreviousMoves(lastMove);
//    freeNode(lastMove);

    lastMove = (Node *) malloc(sizeof(Node));
    if (lastMove == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    lastMove->type = N_DUMMY;
    lastMove->next = NULL;
    lastMove->prev = NULL;

}

/* Revert the active board to the state it was one move before */
Node *undo() {
    if (lastMove->type == N_DUMMY) {
        return NULL;
    } else if (lastMove->type == N_SET) {
        SetMove *set = (SetMove *) lastMove->move;
        board.values[set->x][set->y] = set->old;
        markErrors();
    } else if (lastMove->type == N_AUTOFILL_GENERATE) {
        AutofillGenerateMove *move = (AutofillGenerateMove *) lastMove->move;
        Node *current = move->first;
        while (current != NULL) {
            if (current->type != N_DUMMY) {
                SetMove *set = (SetMove *) current->move;
                board.values[set->x][set->y] = set->old;
            }
            current = current->next;
        }
        markErrors();
    }
    lastMove = lastMove->prev;
    return lastMove->next;
}

/* Redo a move that was undone */
Node *redo() {
    if (lastMove->next == NULL) {
        return NULL;
    } else {
        lastMove = lastMove->next;
        if (lastMove->type == N_SET) {
            SetMove *set = (SetMove *) lastMove->move;
            board.values[set->x][set->y] = set->new;
            markErrors();
        } else if (lastMove->type == N_AUTOFILL_GENERATE) {
            AutofillGenerateMove *move = (AutofillGenerateMove *) lastMove->move;
            Node *current = move->first;
            while (current != NULL) {
                if (current->type != N_DUMMY) {
                    SetMove *set = (SetMove *) current->move;
                    board.values[set->x][set->y] = set->new;
                }
                current = current->next;
            }
            markErrors();
        }
    }
    return lastMove;
}

/* Free a node object from memory */
void freeNode(Node *current) {
    if (current->type == N_AUTOFILL_GENERATE) {
        
    }
    free(current->move);
    free(current);
}
/* Free next move */
void freeNextMove(Node *current) {
    if (current != NULL) {
        freeNextMove(current->next);
        freeNode(current);
    }
}

/* Free all moves after 'start' */
void freeNextMoves(Node *start) {
    freeNextMove(start->next);
}

/* Free all prvious moves before 'start' */
void freePreviousMove(Node *current) {
    if (current != NULL) {
        freePreviousMove(current->prev);
        freeNode(current);
    }
}

/* Free previous move */
void freePreviousMoves(Node *start) {
    freePreviousMove(start->prev);
}


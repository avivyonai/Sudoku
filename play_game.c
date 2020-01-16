#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "play_game.h"
#include "globals.h"
#include "game.h"
#include "ilp_solver.h"
#include "parser.h"
#include "main_aux.h"
#include "backtrack.h"
#include "gurobi_c.h"

/*
 * Module with a function that contains the general logic of the game - gets input from the user and advances the game.
 */

enum STATE {
    S_INIT,
    S_SOLVE,
    S_EDIT
};

/* Main method for the game, gets commands from user and run game logic */
void playGame() {
    enum STATE state = S_INIT;
    int isMarkErrors = 1;
    int tmp;
    char *string = (char *) malloc(sizeof(char));
    Board board, originalBoard;
    int play = 1,i,j;
    Command cmd;
    StringArray *params = (StringArray *) malloc(sizeof(StringArray));
    Node *node;
    Node *current;
    SetMove *setMove = (SetMove *) malloc(sizeof(SetMove));
    AutofillGenerateMove *autofillGenerateMove;

    srand((unsigned) time(NULL));

    if (string == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    if (params == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }
    if (setMove == NULL) {
        printf("ERROR: Could not allocate memory.");
        abort();
    }

    printf("Sudoku\n------\n");
    while (play) {
        printf("Enter your commmand:\n");
        cmd = parseCommand(params);
        switch (cmd) {
            case UNKNOWN:
                printf("ERROR: invalid command\n");
                break;
            case SOLVE:
                if (!params->size) {
                    printf("ERROR: invalid command\n");
                    break;
                }
                if (loadBoard(params->array[0], &originalBoard, cmd == SOLVE)) {
                    initGame();
                    setBoard(originalBoard);
                    board = getBoard();
                    state = S_SOLVE;
                    markErrors();
                    printBoard(getBoard(), isMarkErrors, 1);
                } else {
                    printf("Error: File cannot be opened\n");
                }
                break;
            case EDIT:
                if (!params->size) {
                    initGame();
                    originalBoard = createBoard(3, 3);
                    setBoard(originalBoard);
                    board = getBoard();
                    state = S_EDIT;
                    isMarkErrors = 1;
                    printBoard(getBoard(), isMarkErrors, 0);
                } else if (loadBoard(params->array[0], &originalBoard, cmd == SOLVE)) {
                    initGame();
                    setBoard(originalBoard);
                    board = getBoard();
                    state = S_EDIT;
                    isMarkErrors = 1;
                    markErrors();
                    printBoard(getBoard(), isMarkErrors, 0);
                } else {
                    printf("Error: File cannot be opened\n");
                }
                break;
            case MARK_ERRORS:
                if (state == S_SOLVE) {
                    if (params->size) {
                        if (isDigitsOnly(params->array[0])) {
                            tmp = atoi(params->array[0]);
                            if (tmp == 1 || tmp == 0) {
                                isMarkErrors = tmp;
//                                printBoard(getBoard(), isMarkErrors, 1);
                                break;
                            }
                        }
                        printf("Error: the value should be 0 or 1\n");
                        break;
                    }
                }
                printf("ERROR: invalid command\n");
                break;
            case PRINT_BOARD:
                if (state > 0) {
                    printBoard(getBoard(), isMarkErrors, state == S_SOLVE ? 1 : 0);
                } else {
                    printf("ERROR: invalid command\n");
                }
                break;
            case SET:
                if (state > 0 && params->size > 2) {
                    if (isDigitsOnly(params->array[0]) && isDigitsOnly(params->array[1]) &&
                        isDigitsOnly(params->array[2])) {
                        tmp = setCell((atoi(params->array[0])), (atoi(params->array[1])), atoi(params->array[2]));
                        if (tmp == 0) {
                            printf("Error: cell is fixed\n");
                            break;
                        } else if (tmp == -1) {
                            tmp = board.m * board.n;
                            printf("Error: value not in range 0-%d\n", tmp);
                        } else {
                            board = getBoard();
                            printBoard(board, isMarkErrors, state == S_SOLVE ? 1 : 0);
                            if (state == S_SOLVE && isBoardFull()) {
                                if (isBoardValid()) {
                                    printf("Puzzle solved successfully\n");
                                    state = S_INIT;
                                } else {
                                    printf("Puzzle solution erroneous\n");
                                }
                            }
                        }
                    }
                    else {
                        tmp = board.m * board.n;
                        printf("Error: value not in range 0-%d\n", tmp);
                        break;
                    }
                } else {
                    printf("ERROR: invalid command\n");
                }
                break;
            case VALIDATE:
                if (state > 0) {
                    if (!isBoardValid()) {
                        printf("Error: board contains erroneous values\n");
                        break;
                    }
                    board = getBoard();
                    if (!solve_ilp(&board,0)) {
                        printf("Validation passed: board is solvable\n");
                    } else {
                        printf("Validation failed: board is unsolvable\n");
                    }
                } else {
                    printf("ERROR: invalid command\n");
                }
                break;
            case GENERATE:
                if (state == S_EDIT && params->size > 1) {
                    if (isDigitsOnly(params->array[0]) && isDigitsOnly(params->array[1]) &&
                        atoi(params->array[0]) < countEmptyCells()) {
                        if (generateBoard(atoi(params->array[0]), atoi(params->array[1]))) {
                            printBoard(getBoard(), isMarkErrors, 0);
                        }else {
                            printf("Error: puzzle generator failed\n");
                        }
                    } else {
                        printf("Error: value not in range 0-%d\n", countEmptyCells());
                    }
                } else {
                    printf("ERROR: invalid command\n");
                }
                break;
            case UNDO:
                node = undo();
                if (node == NULL) {
                    printf("Error: no moves to undo\n");
                    break;
                }

                if (node->type == N_SET) {
                    setMove = (SetMove *) node->move;
                    printf("Undo %d,%d: from %c to %c\n", (setMove->y) + 1, (setMove->x) + 1, setMove->new == 0 ? '_' : setMove->new + '0',
                           setMove->old == 0 ? '_' : setMove->old + '0');
                } else if (node->type == N_AUTOFILL_GENERATE) {
                    autofillGenerateMove = (AutofillGenerateMove *) node->move;
                    current = autofillGenerateMove->first;
                    while (current != NULL) {
                        if (current->type != N_DUMMY) {
                            SetMove *set = (SetMove *) current->move;
                            printf("Undo %d,%d: from %c to %c\n", (set->y) + 1, (set->x) + 1, set->new == 0 ? '_' : set->new + '0',
                                   set->old == 0 ? '_' : set->old + '0');

                        }
                        current = current->next;
                    }
                }
                printBoard(getBoard(), isMarkErrors, state == S_SOLVE ? 1 : 0);
                break;
            case REDO:
                node = redo();
                if (node == NULL) {
                    printf("Error: no moves to redo\n");
                    break;
                }

                if (node->type == N_SET) {
                    setMove = (SetMove *) node->move;
                    printf("Redo %d,%d: from %c to %c\n", (setMove->y) + 1, (setMove->x) + 1, setMove->old == 0 ? '_' : setMove->old + '0',
                           setMove->new == 0 ? '_' : setMove->new + '0');
                } else if (node->type == N_AUTOFILL_GENERATE) {
                    autofillGenerateMove = (AutofillGenerateMove *) node->move;
                    current = autofillGenerateMove->first;
                    while (current != NULL) {
                        if (current->type != N_DUMMY) {
                            SetMove *set = (SetMove *) current->move;
                            printf("Redo %d,%d: from %c to %c\n", (set->y) + 1, (set->x) + 1,
                                   set->old == 0 ? '_' : set->old + '0',
                                   set->new == 0 ? '_' : set->new + '0');

                        }
                        current = current->next;
                    }
                }
                printBoard(getBoard(), isMarkErrors, state == S_SOLVE ? 1 : 0);
                break;
            case SAVE:
                if (state > 0 && params->size > 0) {
                    free(string);
                    string = (char *) malloc(sizeof(char));
                    if (string == NULL) {
                        printf("ERROR: Could not allocate memory.");
                        abort();
                    }
                    strcpy(string, params->array[0]);
                    board = getBoard();
                    tmp = 0;
                    if (state == S_EDIT) {
                        if (isBoardValid()) {
                            if (validate(&board)) {
                                tmp = 1;
                                for (i = 0; i < board.size; i++) {
                                    for (j = 0; j < board.size; j++) {
                                        if (board.values[i][j] > 0) {
                                            board.fixedPositions[i][j] = 1;
                                        } else {
                                            board.fixedPositions[i][j] = 0;
                                        }
                                    }
                                }
                            } else {
                                printf("Error: board validation failed\n");
                            }
                        } else {
                            printf("Error: board contains erroneous values\n");
                        }
                    } else if (state == S_SOLVE) {
                        tmp = 1;
                    }
                    if (tmp) {
                        if (saveBoard(board, string)) {
                            printf("Saved to: %s\n", string);
                        } else {
                            printf("Error: File cannot be created or modified\n");
                        }
                    }
                } else {
                    printf("ERROR: invalid command\n");
                }
                break;
            case HINT:
                if (state == S_SOLVE && params->size > 1) {
                    if ((isDigitsOnly(params->array[0]) && isDigitsOnly(params->array[1])
                         && (atoi(params->array[0])<= getBoard().size) && (atoi(params->array[1])<=getBoard().size) )) {
                        tmp = hint(atoi(params->array[0])-1,atoi(params->array[1])-1);
                        board = getBoard();
                        if (!isBoardValid()){
                            printf("Error: board contains erroneous values\n");
                            break;
                        }
                        if (tmp==0 || (!solve_ilp(&board,0) == -1)){
                            printf("Error: board is unsolvable\n");
                            break;
                        }
                        if (tmp == -1) {
                            printf("Error: cell is fixed\n");
                            break;
                        }
                        else if (tmp == -2){
                            printf("Error: cell already contains a value\n");
                            break;
                        }
                        printf("Hint: set cell to %d\n",tmp);
                    } else {
                        printf("Error: value not in range 0-%d\n", board.size);

                    }
                } else {
                    printf("ERROR: invalid command\n");
                }
                break;
            case GUESS_HINT:
                if (state == S_SOLVE && params->size > 1) {
                    if ((isDigitsOnly(params->array[0]) && isDigitsOnly(params->array[1])
                         && (atoi(params->array[0]) < getBoard().size) && (atoi(params->array[1]) < getBoard().size))) {
                        tmp = guess_hint(atoi(params->array[0])-1,atoi(params->array[1])-1);
                        if (!isBoardValid()){
                            printf("Error: board contains erroneous values\n");
                            break;
                        }
                        board = getBoard();
                        if (tmp == 0){
                            printf("Error: board is unsolvable\n");
                            break;
                        }
                        if (tmp == -1) {
                            printf("Error: cell is fixed\n");
                            break;
                        }
                        else if (tmp == -2){
                            printf("Error: cell already contains a value\n");
                            break;
                        }
                    }else {
                        printf("Error: value not in range 0-%d\n", board.size);

                    }
                } else {
                    printf("ERROR: invalid command\n");
                }
                break;
            case GUESS:
                if (state == S_SOLVE && params->size > 0) {
                    if ((atof(params->array[0]) <= 1.0 && (atof(params->array[0]) >= 0.0))) {
                        if (!isBoardValid()){
                            printf("Error: board contains erroneous values\n");
                            break;
                        }
                        tmp = guess(atof(params->array[0]));
                        if (tmp == 0){
                            printf("Error: board is unsolvable\n");
                            break;
                        }
                    }else {
                        printf("Error: value not in range 0-1\n");
                        break;
                    }
                    printBoard(getBoard(), isMarkErrors, 1);
                    printf("Puzzle solved successfully\n");
                    state = S_INIT;
                } else {
                    printf("ERROR: invalid command\n");
                }
                break;
            case NUM_SOLUTIONS:
                if (state > 0) {
                    if (isBoardValid()) {
                        board = getBoard();
                        tmp = numSolutions(&board);
                        printf("Number of solutions: %d\n", tmp);
                        if (tmp == 1) {
                            printf("This is a good board!\n");
                        }
                        else if(tmp ==0){
                            break;
                        }
                        else {
                            printf("The puzzle has more than 1 solution, try to edit it further\n");
                        }
                    } else {
                        printf("Error: board contains erroneous values\n");
                    }
                } else {
                    printf("ERROR: invalid command\n");
                }
                break;
            case AUTOFILL:
                if (state == S_SOLVE) {
                    if (isBoardValid()) {
                        Node *current = autofill();
                        while (current != NULL) {
                            if (current->type != N_DUMMY) {
                                SetMove *set = (SetMove *) current->move;
                                printf("Cell <%d,%d> set to %d\n", (set->y)+1, (set->x)+1, set->new);
                            }
                            current = current->next;
                        }
                        printBoard(getBoard(), isMarkErrors, 1);
                        board = getBoard();
                        if (isBoardFull()){
                            printf("Puzzle solved successfully\n");
                            state = S_INIT;
                        }
                    } else {
                        printf("Error: board contains erroneous values\n");
                    }
                } else {
                    printf("ERROR: invalid command\n");
                }
                break;
            case RESET:
                if (state > 0) {
                    while (undo() != NULL) {
                        continue;
                    }
                    resetMoveList();
                    printf("Board reset\n");
                    printBoard(getBoard(), isMarkErrors, state == S_SOLVE ? 1 : 0);
                } else {
                    printf("ERROR: invalid command\n");
                }
                break;
            case EXIT:
                printf("Exiting...\n");
                free(string);
                freeStringArray(params);
                free(setMove);
                play = 0;
                break;
            case EMPTY:
                break;
        }
    }
}

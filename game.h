#ifndef FINAL_PROJECT_GAME_H
#define FINAL_PROJECT_GAME_H

/* Initializes settings for a new game */
int initGame();

/* Set a board as the active game board */
void setBoard(Board);

/* Creates a Board struct and returns it */
Board createBoard(int, int);

/* Returns active board */
Board getBoard();

/* Returns a copy of the active board */
void getBoardCopy(Board *);

/* Return 1 if the board is full (no value equals to 0) and 0 otherwise */
int isBoardFull();

/* Returns 1 if all values in the board are valid and 0 otherwise */
int isBoardValid();

/* Generates a board by randomly filling X cells, then using ILP to solve the board, and removing Y random cells */
int generateBoard(int, int);

/* Sets a value for a cell in the active board */
int setCell(int, int, int);

/* Returns a possible value that will lead to a solution of the board using ILP */
int hint(int, int);

/* Returns all possible values and scores for a cell using ILP */
int guess_hint(int, int);

int guess(float);

/* Fills all cells that have only one legal value (at the time the function was called) */
Node * autofill();

/* Revert the active board to the state it was one move before */
Node * undo();

/* Redo a move that was undone */
Node * redo();

/* Free all moves after 'start' */
void freeNextMoves(Node *);

/* Returns the number of empty cells in the active board */
int countEmptyCells();

/* Returns 1 if the board is empty (all values are 0) and 0 otherwise */
int isBoardEmpty();

/* Returns a random legal value for a specific cell and board */
int getRandomLegalValue(Board board, int x, int y);

/* checks if given value is valid for a specific cell */
int isCellValid(Board board, int x, int y, int val);

/* Returns the number of possible legal values for a specific cell and board */
int countLegalValues(Board board, int x, int y);

/* Updates the errors marking for the active board - 1 if error exists for cell and 0 otherwise */
void markErrors();

/* Clears all moves */
void resetMoveList();

/* Add a user move the the moves list */
void addMoveToList(NodeType type, void *move);

/* Free a node object from memory */
void freeNode(Node *current);

/* Free previous move */
void freePreviousMoves(Node *start);

#endif /*FINAL_PROJECT_GAME_H */

#ifndef FINAL_PROJECT_BACKTRACK_H
#define FINAL_PROJECT_BACKTRACK_H

/* Returns the number of possible solutions for a given board */
int numSolutions(Board *);

/* return 1 if board is valid */
int validate(Board *);

/* returns no. of sols */
int backtrack(Board *board, int x, int y);

/* checks if given value is valid for a specific cell */
int isValid(Board board, int x, int y, int val);

/* returns an array of possible values for a specific cell */
struct IntArray getPotentialValues(Board board, int x, int y);

#endif /*FINAL_PROJECT_BACKTRACK_H */

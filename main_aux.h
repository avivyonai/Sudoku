#ifndef FINAL_PROJECT_MAIN_AUX_H
#define FINAL_PROJECT_MAIN_AUX_H

/* Prints the board to console in the correct format */
void printBoard(Board, int, int);

/* Saves a given board to a file in a specified path */
int saveBoard(Board, char *);

/* Loads a board saved in a file in the correct format */
int loadBoard(char *, Board *, int);

/* Returns 1 if a string contains only digits and 0 otherwise */
int isDigitsOnly(char *);

#endif /*FINAL_PROJECT_MAIN_AUX_H */

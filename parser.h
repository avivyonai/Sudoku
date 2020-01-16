#ifndef FINAL_PROJECT_PARSER_H
#define FINAL_PROJECT_PARSER_H

typedef enum COMMAND {
    UNKNOWN,
    SOLVE,
    EDIT,
    MARK_ERRORS,
    PRINT_BOARD,
    SET,
    VALIDATE,
    GENERATE,
    UNDO,
    REDO,
    SAVE,
    HINT,
    GUESS_HINT,
    GUESS,
    NUM_SOLUTIONS,
    AUTOFILL,
    RESET,
    EXIT,
    EMPTY
} Command;

/* parser for the user's commands */
Command parseCommand(StringArray *);

/* Returns a Command struct from a given string */
Command getCommandFromString(char *);

#endif /*FINAL_PROJECT_PARSER_H */

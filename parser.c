#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "globals.h"
#include "parser.h"

#define ARRAY_LENGTH(arr) sizeof arr / sizeof arr[0]

static const char *COMMAND_STR[] = { "solve", "edit", "mark_errors", "print_board", "set", "validate", "generate", "undo",
                                     "redo", "save", "hint", "guess_hint", "guess", "num_solutions", "autofill", "reset", "exit" };


/* parser for the user's commands */
Command parseCommand(StringArray *params) {
    const char delim[] = " \t\r\n";
    char input[1024];
    char *value, **tmp;
    int i;
    Command cmd = UNKNOWN;

    value = fgets(input, 1024, stdin);
    value = strtok(input, delim);

    if (value) {
        cmd = getCommandFromString(value);
    } else {
        return EMPTY;
    }
    if (cmd) {
        i = 0;
        tmp = (char**) malloc(MAX_ARRAY_LENGTH * sizeof(char*));
        while ((value = strtok(NULL, delim)) != NULL) {
            tmp[i] = value;
            i++;
        }

        params->size = i;
        params->array = (char**) malloc(params->size * sizeof(char*));
        for (i = 0; i < params->size; i++) {
            params->array[i] = tmp[i];
        }
        free(tmp);
    }

    return cmd;

}

/* Returns a Command struct from a given string */
Command getCommandFromString(char *cmdStr) {
    int i=0;
    int j = (int)ARRAY_LENGTH(COMMAND_STR);
    Command cmd = (Command)0;
    for (i = 0; i < j; i++) {
        if (strcmp(COMMAND_STR[i], cmdStr) == 0) {
            cmd = (Command) (i+1);
            break;
        }
    }
    return cmd;
}

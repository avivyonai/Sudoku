CC = gcc
OBJS = main.o main_aux.o parser.o game.o ilp_solver.o backtrack.o globals.o play_game.o

EXEC = sudoku-console
COMP_FLAGS = -ansi -O3 -Wall -Wextra -Werror -pedantic-errors
GUROBI_COMP = -I/usr/local/lib/gurobi563/include
GUROBI_LIB = -L/usr/local/lib/gurobi563/lib -lgurobi56


$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(GUROBI_LIB) -o $@
main.o: main.c play_game.h
	$(CC) $(COMP_FLAG) -c $*.c
main_aux.o: main_aux.c main_aux.h globals.h game.h
	$(CC) $(COMP_FLAG) -c $*.c
game.o: game.c game.h globals.h backtrack.h ilp_solver.h
	$(CC) $(COMP_FLAG) -c $*.c
play_game.o: game.c play_game.h game.h globals.h backtrack.h ilp_solver.h
	$(CC) $(COMP_FLAG) -c $*.c
ilp_solver.o: ilp_solver.c ilp_solver.h globals.h game.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
parser.o: parser.c parser.h globals.h
	$(CC) $(COMP_FLAG) -c $*.c
backtrack.o: backtrack.c backtrack.h globals.h main_aux.h
	$(CC) $(COMP_FLAG) -c $*.c
globals.o: globals.c globals.h
	$(CC) $(COMP_FLAG) -c $*.c
clean:
	rm -f $(OBJS) $(EXEC)

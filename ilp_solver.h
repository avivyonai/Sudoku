#ifndef FINAL_PROJECT_ILP_SOLVER_H
#define FINAL_PROJECT_ILP_SOLVER_H

int solve_ilp(Board*, int) ;

int solve_lp(Board*, double*) ;

int LPSolveCell(int, int, Board*, float*);

int LPSolver(float, Board*);

#endif /*FINAL_PROJECT_ILP_SOLVER_H */

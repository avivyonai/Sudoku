

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gurobi_c.h"
#include "globals.h"
#include "game.h"
#include "ilp_solver.h"
#include "main_aux.h"


int addvalues(int size,int* ind, double* vals,int *errorptr, GRBmodel *model );
void addConstraints(int size, int blockLength, int blockHeight, int* ind, double* vals,int *errorptr, GRBmodel *model);
int findsol(double* sol, int size, int x, int y);
int ConstraintsLP(int n, int m, int* ind, double* val, GRBmodel *model);
int randomWeightedNumber(double* list, int length);

/* return 0 if borad is solvable, else -1. if save = 1, saves the solves board to board */
int solve_ilp(Board *board, int save) {
    GRBenv *env   = NULL;
    GRBmodel *model = NULL;
    int optimstatus, error = 0,size = board->size, i, j,val;
    int *ind = (int*)calloc(size,sizeof(int));
    double *vals = (double*)calloc(size,sizeof(double));
    double *lb = (double*)calloc(size*size*size,sizeof(double));
    char *vtype = (char*)calloc(size*size*size,sizeof(char));
    char **names = (char**)calloc(size*size*size,sizeof(char*));
    char *namestorage= (char*)calloc(10*size*size*size,sizeof(char));
    char *cursor=NULL;
    double objval;
    double *sol = (double*)calloc(size*size*size,sizeof(double));
    int **copiedVals;
    Board copiedBoard;

    copiedVals = calloc(size, sizeof(int *));
    for (i=0; i<size ;i++) {
        copiedVals[i] = calloc(size, sizeof(int));
    }
    copiedBoard = createBoard(board->m,board->n);
    copyBoard(board,&copiedBoard);
    copiedVals = copiedBoard.values;

    /*decrement values by 1*/
    for(i=0; i<size ; i++) {
        for(j=0; j<size ; j++) {
            copiedVals[i][j] = copiedVals[i][j] - 1;
        }
    }
    /* Create an empty model*/
    cursor = namestorage;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            for (val = 0; val < size; val++) {
                if (copiedVals[i][j] == val) {
                    lb[i*size*size+j*size+val] = 1; /*LB = lowerbound*/
                }
                else {
                    lb[i*size*size+j*size+val] = 0;
                }
                names[i*size*size+j*size+val] = cursor;
                vtype[i*size*size+j*size+val] = GRB_BINARY;
                sprintf(names[i*size*size+j*size+val], "x[%d,%d,%d]", i, j, val+1);
                cursor += strlen(names[i*size*size+j*size+val]) + 1;
            }
        }
    }

    error = GRBloadenv(&env, "sudoku.log");
    if (error) goto QUIT;

    error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
    if (error) goto QUIT;

    /* new model */
    error = GRBnewmodel(env, &model, "sudoku", size*size*size, NULL, lb, NULL, vtype, names);
    if (error) goto QUIT;

    /* set values */
    addvalues(size, ind, vals,&error, model);
    if (error) goto QUIT;

    /* add constraints */
    addConstraints(size, board->n, board->m, ind, vals, &error, model);
    if (error) goto QUIT;

    /* optimize model */
    error = GRBoptimize(model);
    if (error) goto QUIT;

    /* write model to 'sudoku.lp' */
    error = GRBwrite(model, "sudoku.lp");
    if (error) goto QUIT;

    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
    if (error) goto QUIT;
    error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
    if (error) goto QUIT;


    if (optimstatus == GRB_OPTIMAL) {
    }

    else if (optimstatus == GRB_INF_OR_UNBD) {
        goto QUIT;
    }
    else {
        goto QUIT;
    }

    /*save solution to sol*/
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, size*size*size, sol);
    if (error) goto QUIT;

    /*update copiedVals*/
    for(i=0; i<size ; i++) {
        for(j=0; j<size ; j++) {
            copiedVals[i][j] = copiedVals[i][j] + 1;
            if(copiedVals[i][j] == 0) {
                copiedVals[i][j] = findsol(sol, size, i , j);
            }
        }
    }

    if (save == 1) {
        copyBoard(&copiedBoard,board);
    }

    QUIT:
    /* free model*/
    GRBfreemodel(model);
    GRBfreeenv(env);

    /* free copiedVals */
    for (i=0 ; i< size ; i++) {
        free(copiedVals[i]);
    }
    free(copiedVals);

    free(ind); free(lb); free(vtype); free(names); free(namestorage); free(sol); free(vals);
    if (error) {
        return -1;
    }
    return 0;
}

/* return 1 if borad is solvable, else -1 */
int solve_lp(Board *board, double* sol) {

    GRBenv *env   = NULL;
    GRBmodel *model = NULL;

    int optimstatus, error = 0,size = board->size, i, j,val;
    int *ind = (int*)calloc(size,sizeof(int));
    double *vals = (double*)calloc(size,sizeof(double));
    double *lb = (double*)calloc(size*size*size,sizeof(double));
    double *ub = (double*)calloc(size*size*size,sizeof(double)); /* ub = upperbound */
    double *obj = (double*)calloc(size*size*size,sizeof(double)); /* Coefficients for objective's variables */
    char *vtype = (char*)calloc(size*size*size,sizeof(char));
    char **names = (char**)calloc(size*size*size,sizeof(char*));
    char *namestorage= (char*)calloc(10*size*size*size,sizeof(char));
    char *cursor=NULL;
    double objval;
    int **copiedVals;

    copiedVals = board->values;
    /*decrement values by 1*/
    for(i=0; i<size ; i++) {
        for(j=0; j<size ; j++) {
            copiedVals[i][j] = copiedVals[i][j] - 1;
        }
    }


    /* Create an empty model*/
    cursor = namestorage;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            for (val = 0; val < size; val++) {
                if (copiedVals[i][j] == val) {
                    lb[i*size*size+j*size+val] = 1.0;
                    ub[i*size*size+j*size+val] = 1.0;
                    obj[i*size*size+j*size+val] = 1.0;
                }
                else {
                    lb[i*size*size+j*size+val] = 0.0;
                    ub[i*size*size+j*size+val] = 1.0;
                    obj[i*size*size+j*size+val] = 1.0/rand();
                }
                names[i*size*size+j*size+val] = cursor;
                vtype[i*size*size+j*size+val] = GRB_CONTINUOUS;
                sprintf(names[i*size*size+j*size+val], "x[%d,%d,%d]", i, j, val+1);
                cursor += strlen(names[i*size*size+j*size+val]) + 1;
            }
        }
    }

    error = GRBloadenv(&env, "sudoku.log");
    if (error) goto QUIT;

    error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
    if (error) goto QUIT;

    /* new model */
    error = GRBnewmodel(env, &model, "sudoku", size*size*size, obj, lb, ub, vtype, names);
    if (error) goto QUIT;

/*
    addvalues(size, ind, vals,&error, model);
    if (error) goto QUIT;
*/
    /* add constraints */
    ConstraintsLP(board->n, board->m, ind, vals, model);
    if (error) goto QUIT;
/*
    error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MINIMIZE);
    if (error) {
        printf("ERROR %d GRBsetintattr(): %s\n", error, GRBgeterrormsg(env));
        return -1;
    }
*/
    /* optimize model */
    error = GRBoptimize(model);
    if (error) goto QUIT;

    /* write model to 'sudokuLP.lp' */
    error = GRBwrite(model, "sudokuLP.lp");
    if (error) goto QUIT;

    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
    if (error) goto QUIT;

    error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
    if (error) goto QUIT;


    if (optimstatus == GRB_OPTIMAL) {
    }

    else if (optimstatus == GRB_INF_OR_UNBD) {
        goto QUIT;
    }
    else {
        goto QUIT;
    }

    /*save solution to sol*/
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, size*size*size, sol);
    if (error) goto QUIT;


    QUIT:
    /* free model*/
    GRBfreemodel(model);
    GRBfreeenv(env);

    free(ind);
    free(lb);
    free(ub);
    free(vtype);
    free(names);
    free(namestorage);
    free(obj);
    free(vals);

    if (error) {
        return -1;
    }


    return 1;
}

/*running LP to get scores for a specific cell - guess_hint func */
int LPSolveCell(int x, int y, Board *board, float* legal_options){
    /* Linear programming algorithm, Used by guess and guessHint functions. solves using gorubi solver module
    *  args: gets board to solve, doesn't change it, builds solution on guess board
    *  return: all possible solutions with scores higher than 0 and their scores to specific cell, 0 -> no solution to guess_board, 1 -> there is solution
    */
    int success, i, sol_index;
    const int size = board->size;
    double*	sol = (double*)calloc(size*size*size,sizeof(double));
    success = solve_lp(board,sol);
    if(!success){
        free(sol);
        return 0;
    }
    sol_index = size*size*(x-1) + size*(y-1);
    for (i=0; i<size; i++) {
        if(sol[sol_index+i]>0){
            legal_options[i*2]= i+1.0;
            legal_options[i*2+1]= sol[sol_index+i];
       }
        else{
            legal_options[i*2]= -1.0;
            legal_options[i*2+1]= -1.0;

        }
    }

    free(sol);
    return 1;

}

/*running LP to get scores for a complete board - guess func */
int LPSolver(float threshold, Board *board){
    int success, valid_cnt, sol_index, i, j, v, valid, sol_value;
    const int size=board->size;
    double*	sol;
    double* legal_options;
    sol = (double*)calloc(size*size*size,sizeof(double));
    legal_options = (double*)calloc(size*2,sizeof(double));
    success = solve_lp(board,sol);
    if(!success){
        free(sol);
        free(legal_options);
        return 0;
    }

    /*increment all values of board in 1 */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            board->values[i][j] = board->values[i][j] +1;
        }
    }

    valid_cnt = 0;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            valid_cnt = 0;
            for (v = 0; v < size; v++) {
                sol_index = i*size*size+j*size+v;
                if (sol[sol_index] > threshold && board->values[i][j]==0) {
                    board->values[i][j] = v+1;
                    valid = isCellValid(*board,i,j,v+1);
                    if(valid){
                        legal_options[valid_cnt*2]= (double)v+1.0; /*optional value*/
                        legal_options[valid_cnt*2+1]= sol[sol_index]; /*score*/
                        valid_cnt++;
                    }
                    board->values[i][j] = 0;
                }
            }
            sol_value = randomWeightedNumber(legal_options, valid_cnt);
            if(sol_value!=-1){
                board->values[i][j] = sol_value;
                setCell(i,j,sol_value);
            }
            }
        }


    printBoard(*board,0,0);
    free(sol);
    free(legal_options);
    return 1;

}



int addvalues(int size, int* ind, double* vals, int *errorptr, GRBmodel *model ){
    int i,j,val;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            for (val = 0; val < size; val++) {
                ind[val] = i*size*size + j*size + val; /*ind[val] equals the variable's index*/
                vals[val] = 1.0; /*vals[val] equals the variable index*/
            }
            *errorptr = GRBaddconstr(model, size, ind, vals, GRB_EQUAL, 1.0, NULL);
            if (*errorptr) {
                return 1;
            }
        }
    }
    return 0;
}

/* constraints for the ILP model */
void addConstraints(int size, int blockLength, int blockHeight, int* ind, double* vals,int *errorptr, GRBmodel *model){
    /* row constraints */
    int i,j,v,a,b,cnt;
    for (v = 0; v < size; v++) {
        for (j = 0; j < size; j++) {
            for (i = 0; i < size; i++) {
                ind[i] = i*size*size + j*size + v;
                vals[i] = 1.0;
            }
            *errorptr = GRBaddconstr(model, size, ind, vals, GRB_EQUAL, 1.0, NULL);
            if (*errorptr) return;
        }
    }

    if (*errorptr) return;

    /* column constraints */
    for (v = 0; v < size; v++) {
        for (i = 0; i < size; i++) {
            for (j = 0; j < size; j++) {
                ind[j] = i*size*size + j*size + v;
                vals[j] = 1.0;
            }
            *errorptr = GRBaddconstr(model, size, ind, vals, GRB_EQUAL, 1.0, NULL);
            if (*errorptr) return;
        }
    }
    if (*errorptr) return;

    /* block constraints */
    for (v = 0; v < size; v++) {
        for (a = 0; a < blockLength; a++) {
            for (b = 0; b < blockHeight; b++) {
                cnt = 0;
                for (i = a*blockHeight; i < (a+1)*blockHeight; i++) {
                    for (j = b*blockLength; j < (b+1)*blockLength; j++) {
                        ind[cnt] = i*size*size + j*size + v;
                        vals[cnt] = 1.0;
                        cnt++;
                    }
                }
                *errorptr = GRBaddconstr(model, size, ind, vals, GRB_EQUAL, 1.0, NULL);
                if (*errorptr) return;
            }
        }
    }

}

/* get a specific cell solution from ILP */
int findsol(double* sol, int size, int x, int y) {
    int i;
    int start =	size*size*x + size*y;
    for (i=0; i<size; i++) {
        if ((int)sol[start+i] == 1) {
            return i+1;
        }
    }
    return -1;
}

/* constraints for the LP model */
int ConstraintsLP(int n, int m, int* ind, double* val, GRBmodel *model){

/*
	 the function adds to model all the constraints of Sudoku game
	 * return 0 --> error, 1---> success
	 First constraint: Each cell gets a value
		   variables xij0...xij(N-1) (0,1,..,N-1)
		  ind[0] =  i*N*N + j*N + 0; ind[1] = i*N*N + j*N + 1; ...; ind[2] =  i*N*N + j*N + N-1;
		  / coefficients (according to variables in "ind")
		  val[0] = 1; val[1] = 1;... ;val[N-1] = 1;
*/

    int i, j, v, error, error2, ig, jg, count;
    GRBenv   *env   = NULL;
    const int N= n*m;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            for (v = 0; v < N; v++) {
                ind[v] = i*N*N + j*N + v;
                val[v] = 1.0;
            }
            error = GRBaddconstr(model, N, ind, val, GRB_LESS_EQUAL, 1.0, NULL);
            error2 = GRBaddconstr(model, N, ind, val, GRB_GREATER_EQUAL, 0.0, NULL);
            if (error) {
                printf("ERROR %d Each cell gets a value GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                return 1;
            }
            if (error2) {
                printf("ERROR %d Each cell gets a value GRBaddconstr(): %s\n", error2, GRBgeterrormsg(env));
                return 1;
            }
        }
    }


    /* Second constraint: Each value must appear once in each row*/
    for (v = 0; v < N; v++) {
        for (j = 0; j < N; j++) {
            for (i = 0; i < N; i++) {
                ind[i] = i*N*N + j*N + v;
                val[i] = 1.0;
            }

            error = GRBaddconstr(model, N, ind, val, GRB_EQUAL, 1.0, NULL);
            if (error) {
                printf("ERROR %d Each value must appear once in each row GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                return 1;
            }
        }
    }

    /*   Third constraint: Each value must appear once in each column*/
    for (v = 0; v < N; v++) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                ind[j] = i*N*N + j*N + v;
                val[j] = 1.0;
            }

            error = GRBaddconstr(model, N, ind, val, GRB_EQUAL, 1.0, NULL);
            if (error) {
                printf("ERROR %d Each value must appear once in each column GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                return 1;
            }
        }
    }

    /*  Fourth constraint: Each value must appear once in each subgrid*/

    for (v = 0; v < N; v++) {
        for (ig = 0; ig < n; ig++) { /*n-> block length*/
            for (jg = 0; jg < m; jg++) { /* m-> block width*/
                count = 0;
                for (i = ig*m; i < (ig+1)*m; i++) {
                    for (j = jg*n; j < (jg+1)*n; j++) {
                        ind[count] = i*N*N + j*N + v;
                        val[count] = 1.0;
                        count++;
                    }
                }

                error = GRBaddconstr(model, N, ind, val, GRB_EQUAL, 1.0, NULL);
                if (error) {
                    printf("ERROR %d Each value must appear once in each subgrid GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* return random v out of list with values and their weights. return -1 in case of an error;*/
int randomWeightedNumber(double* list, int length){
    double sum_of_weight, rnd;
    int location,i;
    sum_of_weight = 0.0;
    for(i=0; i<length; i++) {
        location = i*2+1;
        sum_of_weight += list[location];
    }
    rnd = (double)rand()/RAND_MAX* sum_of_weight;
    for(i=0; i<length; i++) {
        location = i*2+1;
        if(rnd < list[location])
            return (int)list[location-1];
        rnd -= list[location];
    }
    return -1;
}



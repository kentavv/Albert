#ifndef _CREATEMATRIX_H_
#define _CREATEMATRIX_H_

/*******************************************************************/
/***  FILE :        CreateMatrix.h                               ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/*******************************************************************/

#include "Sparse_structs.h"
#include "Sparse_defs.h"

typedef Scalar *Matrix;

typedef struct {
    Scalar coef;
    Basis left_basis;
    Basis right_basis;
} Basis_pair;

typedef struct {
    Basis left_basis;
    Basis right_basis;
} Unique_basis_pair,*Unique_basis_pair_list;

typedef struct eqn_list_node{
    Basis_pair *basis_pairs;
    struct eqn_list_node *next;
} Eqn_list_node; 

int CreateTheMatrix(Eqn_list_node *Eq_list, Matrix *Mptr, int *Rows, int *Cols, Unique_basis_pair_list *BPCptr, Name n);
int SparseCreateTheMatrix(Eqn_list_node *Eq_list, MAT_PTR *SMptr, int *Rows, int *Cols, Unique_basis_pair_list *BPCptr, Name n);
void DestroyBPtoCol(void);
void DestroyTheMatrix(void);
int GetCol(Basis Left_basis, Basis Right_basis);

#endif

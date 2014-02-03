#ifndef _CREATEMATRIX_H_
#define _CREATEMATRIX_H_

/*******************************************************************/
/***  FILE :        CreateMatrix.h                               ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/*******************************************************************/

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

#endif

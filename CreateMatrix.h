#ifndef _CREATEMATRIX_H_
#define _CREATEMATRIX_H_

/*******************************************************************/
/***  FILE :        CreateMatrix.h                               ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/*******************************************************************/

#include <list>
#include <vector>

#include "Build_defs.h"

struct Node {
  int column;
  Scalar element;
};

typedef Scalar *Matrix;
typedef std::list<Node> SparseRow;
typedef std::vector<SparseRow> SparseMatrix;

typedef struct {
    Scalar coef;
    Basis left_basis;
    Basis right_basis;
} Basis_pair;

typedef struct {
    Basis left_basis;
    Basis right_basis;
} Unique_basis_pair;

struct Eqn_list_node{
    Basis_pair *basis_pairs;
    Eqn_list_node *next;
}; 

int CreateTheMatrix(Eqn_list_node *Eq_list, Matrix *Mptr, int *Rows, int *Cols, std::vector<Unique_basis_pair> &BPCptr, Name n);
int SparseCreateTheMatrix(Eqn_list_node *Eq_list, SparseMatrix &SM, int *Rows, int *Cols, std::vector<Unique_basis_pair> &BPCptr, Name n);
void DestroyTheMatrix(void);
int GetCol(const std::vector<Unique_basis_pair> &ColtoBP, Basis Left_basis, Basis Right_basis);

#endif

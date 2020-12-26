#ifndef _SPARSE_REDUCE_MATRIX7_H_
#define _SPARSE_REDUCE_MATRIX7_H_

#include "CreateMatrix.h"

int SparseReduceMatrix7(SparseMatrix &SM, int nCols, int *Rank);
Scalar Get_Matrix_Element(const SparseMatrix &SM, int i, int j);

#endif

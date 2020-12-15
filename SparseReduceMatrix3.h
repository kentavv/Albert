#ifndef _SPARSE_REDUCE_MATRIX3_H_
#define _SPARSE_REDUCE_MATRIX3_H_

#include "CreateMatrix.h"

int SparseReduceMatrix3(SparseMatrix &SM, int nCols, int *Rank);
Scalar Get_Matrix_Element3(const SparseMatrix &SM, int i, int j);

#endif

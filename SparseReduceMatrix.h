#ifndef _SPARSE_REDUCE_MATRIX_H_
#define _SPARSE_REDUCE_MATRIX_H_

#include <list>
#include <vector>

#include "CreateMatrix.h"

int SparseReduceMatrix(std::vector<std::list<Node> > &SM, int Rows, int Cols, int *Rank);
Scalar Get_Matrix_Element(const std::vector<std::list<Node> > &SM, int i, int j);

#endif

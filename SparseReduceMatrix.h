#ifndef _SPARSE_REDUCE_MATRIX_H_
#define _SPARSE_REDUCE_MATRIX_H_

#include "Build_defs.h"
#include "Sparse_structs.h"
#include "Sparse_defs.h"

int SparseReduceMatrix(MAT_PTR *Matrix_BPtr, int Rows, int Cols, int *Rank);
Scalar Get_Matrix_Element(MAT_PTR Sparse_Matrix, int i, int j);
void Insert_Element(MAT_PTR Sparse_Matrix, int matrow, Scalar element, int column, NODE_PTR Prev_Ptr);
void Delete_Element(MAT_PTR Sparse_Matrix, int RowId, NODE_PTR Prev_Ptr);
void Change_Element(NODE_PTR Element_Ptr, Scalar value);
int Row_empty(MAT_PTR Sparse_Matrix, int row);
NODE_PTR Locate_Node(MAT_PTR Sparse_Matrix, int row, int col);

#endif

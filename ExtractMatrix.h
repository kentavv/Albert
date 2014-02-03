#ifndef _EXTRACT_MATRIX_H_
#define _EXTRACT_MATRIX_H_

#include "Build_defs.h"
#include "CreateMatrix.h"

int ExtractFromTheMatrix(Matrix Mptr, int Rows, int Cols, int Rank, Name N, Unique_basis_pair_list BPCptr);
int SparseExtractFromMatrix(MAT_PTR SMptr, int Rows, int Cols, int Rank, Name N, Unique_basis_pair_list BPCptr);

#endif


#ifndef _EXTRACT_MATRIX_H_
#define _EXTRACT_MATRIX_H_

#include "Build_defs.h"
#include "CreateMatrix.h"

int ExtractFromTheMatrix(const Matrix Mptr, int nRows, int nCols, int Rank, Name N, const std::vector<Unique_basis_pair> &ColtoBP);
int SparseExtractFromMatrix(const SparseMatrix &SM, int nRows, int nCols, int Rank, Name N, const std::vector<Unique_basis_pair> &ColtoBP);

#endif


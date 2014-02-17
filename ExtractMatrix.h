#ifndef _EXTRACT_MATRIX_H_
#define _EXTRACT_MATRIX_H_

#include <list>
#include <vector>

#include "Build_defs.h"
#include "CreateMatrix.h"

int ExtractFromTheMatrix(Matrix Mptr, int Rows, int Cols, int Rank, Name N, const std::vector<Unique_basis_pair> &ColtoBP);
int SparseExtractFromMatrix(std::vector<std::list<Node> > &SM, int Rows, int Cols, int Rank, Name N, const std::vector<Unique_basis_pair> &ColtoBP);

#endif


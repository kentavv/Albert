#ifndef _GENERATE_EQUATIONS_H_
#define _GENERATE_EQUATIONS_H_

#include <vector>

#include "Build_defs.h"
#include "CreateMatrix.h"
#include "Po_parse_exptext.h"

int GenerateEquations(const struct polynomial *F, Name N, Equations &equations, SparseMatrix &SM, int &cols, std::vector<Unique_basis_pair> &BPtoCol);
int GetVarNumber(char Letter);

#endif

#ifndef _CREATE_SUBS_
#define _CREATE_SUBS_

#include <vector>

#include "Build_defs.h"
#include "CreateMatrix.h"
#include "Po_parse_exptext.h"

int CreateSubs(Equations &equations, const struct polynomial *F, int nVars, int Mdv, const std::vector<std::vector<Basis> > &all_Substitutions, const int *Deg_var_types);
void BuildSubs(const std::vector<Name> &Set_partitions, int maxDegVar, const int *Deg_var, int row, int col, std::vector<Basis> &tmp, int nVars, std::vector<std::vector<Basis> > &Substitutions);

#endif

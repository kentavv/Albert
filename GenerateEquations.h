#ifndef _GENERATE_EQUATIONS_H_
#define _GENERATE_EQUATIONS_H_

#include "Build_defs.h"
#include "CreateMatrix.h"
#include "Po_parse_exptext.h"

int GenerateEquations(const struct polynomial *F, Name N, Equations &equations);
int GetVarNumber(char Letter);

#endif

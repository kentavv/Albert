#ifndef _GENERATORS_H_
#define _GENERATORS_H_

#include "Build_defs.h"
#include "Po_parse_exptext.h"

int Parse_generator_word(char Str[], struct P_type *Pntr);
void AssignBasisNumberstoLetters(struct P_type ptype);
char GetLetterofBasis(Basis b);
Basis GetBasisNumberofLetter(char c);

#endif


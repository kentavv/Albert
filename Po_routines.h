#ifndef _PO_ROUTINES_H_
#define _PO_ROUTINES_H_

#include "Po_parse_exptext.h"

void Print_poly(struct polynomial *Poly, int Poly_len);
int Homogeneous(struct polynomial *Poly);
void AssignNumbersToLetters(struct polynomial *Poly);
void DestroyPoly(struct polynomial *Poly);
int IsIdentity(struct polynomial *Poly);

#endif

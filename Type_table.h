#ifndef _TYPE_TABLE_H_
#define _TYPE_TABLE_H_

/*******************************************************************/
/***  FILE :        Type_table.h                                 ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/*******************************************************************/

#include "Build_defs.h"

typedef struct tt_node {
    Basis begin_basis;
    Basis end_basis;            /* indices into Base table. */
    Type type;
} TT_node; 

int CreateTypeTable(Type Cur_type);
int GetTargetLen(void);
Type GetNewType(void);
void NameToType(Name N, Type T);
void SubtractTypeName(Name n1, Name n2, Name *res_name);
int GetDegree(Type Pntr);
int GetDegreeName(Name n);
Name TypeToName(Type T);
int IsSubtype(Name n1, Name n2);
void EnterEndBasis(int TTindex, Basis basis);
void UpdateTypeTable(Name n, Basis Begin_basis, Basis End_basis);
void DestroyTypeTable(void);
Name FirstTypeDegree(Degree D);
Name NextTypeSameDegree(Name n);
Basis FirstBasis(Name N);
Basis NextBasisSameType(Basis B);
Basis BeginBasis(Name n);
Basis EndBasis(Name n);

#endif

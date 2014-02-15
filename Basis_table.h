#ifndef _BASIS_TABLE_H_
#define _BASIS_TABLE_H_

/*******************************************************************/
/***  FILE :        Basis_table.h                                ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/***  MODIFIED:  9/93 - Trent Whiteley                           ***/
/***                    changed Basis_table from array to ptr    ***/
/*******************************************************************/

#include <stdio.h>

#include "Build_defs.h"

typedef struct {
    Basis left_factor;
    Basis right_factor;
    Name type;
} BT_rec; 

typedef struct {
    Basis first_basis;
    Basis last_basis;
} Deg_to_basis_rec; 

void initBasisTable(void);
void freeBasisTable(void);
int basisTableReady(void);

int CreateBasisTable(int Target_degree);
Basis EnterBasis(Basis Left_factor, Basis Right_factor, Name Cur_type);
Basis GetNextBasisTobeFilled(void);
Basis BasisStart(Degree Deg);
Basis BasisEnd(Degree Deg);
Name GetType(Basis B);
void PrintBasisTable(FILE *filePtr, int outputType);

#endif

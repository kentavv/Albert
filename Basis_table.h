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

int CreateBasisTable();
Basis EnterBasis(Basis Left_factor, Basis Right_factor, Name Cur_type);
Basis GetNextBasisTobeFilled();
Basis BasisStart(Degree Deg);
Basis BasisEnd(Degree Deg);
Name GetType(Basis B);
void PrintBasisTable(FILE *filePtr);

bool save_basis_table(FILE *f);
bool restore_basis_table(FILE *f);

#endif

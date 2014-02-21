/*******************************************************************/
/***  FILE :        Basis_table.c                                ***/
/***  AUTHOR:       David P Jacobs                               ***/
/***  PROGRAMMER:   Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990.                                    ***/
/***  MODIFIED:  9/93 - Trent Whiteley                           ***/
/***                    Added support for view, output, & save   ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      int CreateBasisTable()                                 ***/
/***      Basis EnterBasis()                                     ***/
/***      int GetNextbasistobefilled()                           ***/
/***      Basis LeftFactor()                                     ***/
/***      Basis RightFactor()                                    ***/
/***      int Getdeg()                                           ***/
/***      Type  GetType()                                        ***/
/***  PRIVATE ROUTINES:                                          ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines dealing with Basis Table.***/
/*******************************************************************/

#include <vector>

using std::vector;
using std::pair;
using std::make_pair;

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "Basis_table.h"
#include "Build_defs.h"
#include "Generators.h"
#include "Help.h"
#include "Memory_routines.h"
#include "Type_table.h"

static void UpdateDegToBasisTable(int Deg, Basis Cur_basis);
static void PrintBasis(Basis b, FILE *filePtr);

typedef struct {
    Basis left_factor;
    Basis right_factor;
    Name type;
} BT_rec;

static vector<BT_rec> Basis_table;
static vector<pair<Basis, Basis> > Deg_to_basis_table; // maps degree to first and last indices in Basis_table associated with that degree

/*******************************************************************/
/* GLOBALS INITIALIZED:                                            */
/*     Basis_table -- to zeroes.                                   */
/*     Nextbasistobefilled -- to 1.                                */
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Initialize the Basis Table and Nextbasistobefilled.         */
/*******************************************************************/ 
int CreateBasisTable()
{
    Basis_table.clear();
    { // First entry is expected to be zero, but this may not be required
      BT_rec br;
      br.left_factor = 0;
      br.right_factor = 0;
      br.type = 0; 
      Basis_table.push_back(br);
    }

    Deg_to_basis_table.clear();

    return(OK);
}
    

/*******************************************************************/
/* GLOBALS MODIFIED:                                               */
/*     Basis_table -- New Basis is entered.                        */ 
/*     Nextbasistobefilled -- Incremented.                         */
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Left_factor -- of the new Basis.                            */
/*     Right_factor -- of the new Basis.                           */
/*     Type -- of the new Basis.                                   */
/* RETURNS:                                                        */
/*     Basis Number of the new Basis entered.                      */
/* FUNCTION:                                                       */
/*     Enter a New Basis into Basis Table.                         */
/*******************************************************************/ 
Basis EnterBasis(Basis Left_factor, Basis Right_factor, Name Cur_type)
{
    Basis bn = Basis_table.size();

    BT_rec br;
    br.left_factor = Left_factor;
    br.right_factor = Right_factor;
    br.type = Cur_type; 
    Basis_table.push_back(br);

    UpdateDegToBasisTable(GetDegreeName(Cur_type), bn);

    return bn;
}


void UpdateDegToBasisTable(int Deg, Basis Cur_basis)
{
    if(Deg > (int)Deg_to_basis_table.size()) {
      Deg_to_basis_table.push_back(make_pair(Cur_basis, Cur_basis));
    } else {
      Deg_to_basis_table[Deg-1].second++;
    }
}

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Nextbasistobefilled --                                      */
/*******************************************************************/ 
Basis GetNextBasisTobeFilled(void)
{
    return Basis_table.size();
}


Basis BasisStart(Degree Deg)
{
    return(Deg_to_basis_table[Deg-1].first);
}


Basis BasisEnd(Degree Deg)
{
    return(Deg_to_basis_table[Deg-1].second);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     B -- Basis element.                                         */
/* RETURNS:                                                        */
/*     Type of the Basis B.                                        */
/*******************************************************************/ 
Name GetType(Basis B)
{
    return(Basis_table[B].type);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*    FILE *filePtr - pointer to output file, temp file, or stdout */ 
/*    int outputType - indicates screen, file, or printer          */
/* RETURNS:                                                        */
/*     Type of the Basis B.                                        */
/*******************************************************************/ 
void PrintBasisTable(FILE *filePtr) /* TW 9/19/93 - added 2 params to support view, save, & output */
{
  if(Basis_table.size() > 1){
    fprintf(filePtr, "Basis Table: \n");
    for (int i=1; i<(int)Basis_table.size(); i++) {
         fprintf(filePtr, " %3d.   %3d %3d   ",i,Basis_table[i].left_factor,
                             Basis_table[i].right_factor);
         PrintTypeName(Basis_table[i].type, filePtr);
         fprintf(filePtr, "    ");
         PrintBasis(i, filePtr);
         fprintf(filePtr, "\n");
    }
  }
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     B -- Basis element.                                         */
/* RETURNS:                                                        */
/*     Type of the Basis B.                                        */
/*******************************************************************/ 
void PrintBasis(Basis b, FILE *filePtr) /* TW 9/19/93 - added param to support view, save, & output */
{

    if ( (Basis_table[b].left_factor == 0) && 
         (Basis_table[b].right_factor == 0) ) {
        fprintf(filePtr, "%c", GetLetterofBasis(b) );
    } else {
        fprintf(filePtr, "(");
        PrintBasis(Basis_table[b].left_factor, filePtr);
        PrintBasis(Basis_table[b].right_factor, filePtr);
        fprintf(filePtr, ")");
    }
}

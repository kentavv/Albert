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

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "Basis_table.h"
#include "Build_defs.h"
#include "Generators.h"
#include "Help.h"
#include "Memory_routines.h"
#include "Type_table.h"

static int InitializeDegtoBasisTable(int Target_degree);
static void UpdateDegToBasisTable(int Deg, Basis Cur_basis);
#if 0
static Basis LeftFactor(Basis B);
static Basis RightFacto(Basis B);
static int GetDeg(Basis B);
#endif
static void PrintBasis(Basis b, FILE *filePtr);

static int Nextbasistobefilled;
static int Deg_of_last_basis;
Deg_to_basis_rec *Deg_to_basis_table = NULL;
extern jmp_buf env;

/* TW 9/25/93 - line counter for view */
int lineCnt;

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
int CreateBasisTable(int Target_degree)
{
    int i;

    for (i=0;i<(DIMENSION_LIMIT + 1);i++) { 
          Basis_table[i].left_factor = 0;
          Basis_table[i].right_factor = 0;
          Basis_table[i].type = 0; 
    }

    if (Deg_to_basis_table != NULL)
        free(Deg_to_basis_table);

    if (InitializeDegtoBasisTable(Target_degree) != OK)
        return(0);
    Nextbasistobefilled = 1;
    Deg_of_last_basis = 0;
    return(OK);
}
    

int InitializeDegtoBasisTable(int Target_degree)
{
    int i;

    Deg_to_basis_table = (Deg_to_basis_rec *) (Mymalloc(Target_degree *
                                          sizeof(Deg_to_basis_rec)));
    assert_not_null(Deg_to_basis_table);

    /*  
      5/94 (DPJ):  initialize all of Deg_to_basis_table 
                   rather than just first entry.
    */

    for (i=0; i < Target_degree; i++) {			
       Deg_to_basis_table[i].first_basis = 0;
       Deg_to_basis_table[i].last_basis = 0;
    }

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
    int i = Nextbasistobefilled;

    if (i > DIMENSION_LIMIT) {
        fprintf(stderr,"DIMENSION LIMIT %d exceeded. \n",i-1);
        longjmp(env,1);
    }
    Basis_table[i].left_factor = Left_factor;
    Basis_table[i].right_factor = Right_factor;
    Basis_table[i].type = Cur_type; 
    UpdateDegToBasisTable(GetDegreeName(Cur_type),(Basis) i);
    return(Nextbasistobefilled++);
}


void UpdateDegToBasisTable(int Deg, Basis Cur_basis)
{
    if (Deg > Deg_of_last_basis) {
        Deg_to_basis_table[Deg-1].first_basis = Cur_basis;
        Deg_to_basis_table[Deg-1].last_basis = Cur_basis;
        Deg_of_last_basis = Deg;
    }
    else
        (Deg_to_basis_table[Deg-1].last_basis)++;
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Nextbasistobefilled --                                      */
/*******************************************************************/ 
Basis GetNextBasisTobeFilled(void)
{
    return(Nextbasistobefilled);
}

#if 0
/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     B -- Basis element.                                         */
/* RETURNS:                                                        */
/*     Left factor of the Basis element.                           */ 
/*******************************************************************/ 
Basis LeftFactor(Basis B)
{
    return(Basis_table[B].left_factor);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     B -- Basis element.                                         */
/* RETURNS:                                                        */
/*     Right factor of the Basis element.                          */ 
/*******************************************************************/ 
Basis RightFacto(Basis B)
{
    return(Basis_table[B].right_factor);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     B -- Basis element.                                         */
/* RETURNS:                                                        */
/*     Degree of the Basis element.                                */ 
/*******************************************************************/ 
int GetDeg(Basis B)
{
    return(GetDegreeName(Basis_table[B].type));
}
#endif


Basis BasisStart(Degree Deg)
{
    return(Deg_to_basis_table[Deg-1].first_basis);
}


Basis BasisEnd(Degree Deg)
{
    return(Deg_to_basis_table[Deg-1].last_basis);
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
void PrintBasisTable(FILE *filePtr, int outputType) /* TW 9/19/93 - added 2 params to support view, save, & output */
{
    int i;

  if(Nextbasistobefilled > 0){
    fprintf(filePtr, "Basis Table: \n");
    ++lineCnt;			/* TW 9/19/93 - support for view */
    for (i=1;i<Nextbasistobefilled;i++) {
         fprintf(filePtr, " %3d.   %3d %3d   ",i,Basis_table[i].left_factor,
                             Basis_table[i].right_factor);
         PrintTypeName(Basis_table[i].type, filePtr);
         fprintf(filePtr, "    ");
         PrintBasis(i, filePtr);
         fprintf(filePtr, "\n");
	 if(outputType == 1){	/* TW 9/19/93 - support for view */
	   ++lineCnt;
	   more(&lineCnt);
	 }
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
         (Basis_table[b].right_factor == 0) )
        fprintf(filePtr, "%c", GetLetterofBasis(b) );
    else {
        fprintf(filePtr, "(");
        PrintBasis(Basis_table[b].left_factor, filePtr);
        PrintBasis(Basis_table[b].right_factor, filePtr);
        fprintf(filePtr, ")");
    }
}

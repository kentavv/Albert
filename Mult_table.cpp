/*******************************************************************/
/***  FILE :        Mult_table.c                                 ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990.                                    ***/
/***  MODIFICATION:  9/93 - Trent Whiteley                       ***/
/***                        added routines getMtBlock() and      ***/
/***                        setMtBlock() in order to dynamically ***/
/***                        allocate the multiplication table    ***/
/***                        and added code to free it up later   ***/
/***                        also added code to support the save, ***/
/***                        view, and output commands            ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      Termptr RetrieveProduct()                              ***/
/***      int EnterProduct()                                     ***/
/***      int CreateMultTable()                                  ***/
/***      int DestroyMultTable()                                 ***/
/***      Mt_block *Alloc_Mt_block()                             ***/
/***      Terms_block *Alloc_Terms_block()                       ***/
/***  PRIVATE ROUTINES:                                          ***/
/***      int TermsListLength()                                  ***/
/***      int FreeTermsBlocks()                                  ***/ 
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines dealing with Algebraic   ***/
/***      elements.                                              ***/
/*******************************************************************/

#include <map>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Mult_table.h"
#include "Build_defs.h"
#include "Alg_elements.h"
#include "Help.h"
#include "Memory_routines.h"
#include "Scalar_arithmetic.h"
#include "Basis_table.h"

//using std::map;
using namespace std;

map<pair<Basis, Basis>, vector<pair<Basis, Scalar> > > mult_table;

static void Print_AE(const Alg_element &ae, FILE *filePtr, int outputType);

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* GLOBALS REQUIRED:                                               */
/*     Mt_block_index -- Translation table.                        */ 
/*     First_terms_block                                           */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Free the memory occupaid by Multiplication table and        */
/*     all the Terms_blocks.                                       */
/*******************************************************************/ 
void DestroyMultTable(void)
{
  mult_table.clear();
}


/*******************************************************************/
/* GLOBALS MODIFIED:                                               */
/*     None.                                                       */
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     FILE *filePtr - a ptr to output file, temp file, or stdout  */
/*     int outputType - output to printer, screen, or a file       */
/* RETURNS:                                                        */
/*     Nothing                                                     */
/* FUNCTION:                                                       */
/*     Print the table.                                            */
/*******************************************************************/
void Print_MultTable(FILE *filePtr, int outputType) /* TW 9/19/93 - added 2 params to support view, save, & output */
{
  int i, j, dim;
  Alg_element prod;

  dim = GetNextBasisTobeFilled();
  if(dim > 0){
    fprintf(filePtr, "\nMultiplication table: \n");
    for (i = 1; i <= dim; i++) {
      for (j = 1; j <= dim; j++) {
        prod.clear();
        Mult2basis(i, j, 1, prod);	/* TW 9/22/93 - change prod[] to *prod */
        if (!IsZeroAE(prod)) {	/* TW 9/22/93 - change prod[] to *prod */
          /* PrintBasis(i); */
          fprintf(filePtr, "(b%d)*(b%d)\n",i,j);
          /* PrintBasis(j); */
          Print_AE(prod, filePtr, outputType); 	/* TW 9/22/93 - change prod[] to *prod */
          fprintf(filePtr, "\n\n");
        }
      }
    }   
  }
}
 
void Print_AE(const Alg_element &ae, FILE *filePtr, int outputType) /* TW 9/19/93 - add 2 params to support view, save, & output */
{
  int trmcnt = 0;		/* How many terms have been printed */
  int lnecnt = 0;		/* How many have been printed on current line */

  map<Basis, Scalar>::const_iterator aei;
  for(aei = ae.begin(); aei != ae.end(); aei++) {
    if(aei->first != 0 && aei->second != 0) {
      int x = aei->second; 
      if ((trmcnt > 0) && (trmcnt % 4 == 0)) { 		/* 4 items per line */
        fprintf(filePtr, "\n");	
        lnecnt = 0;
      }
      if (lnecnt == 0){
	fprintf(filePtr, "   %3d b%-4d", x, aei->first);
      } else{
        fprintf(filePtr, "+  %3d b%-4d", x, aei->first);
      }
      trmcnt++;
      lnecnt++;
    }
  }
}


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

mult_table_t mult_table;

static void Print_AE(const Alg_element &ae, FILE *filePtr);

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
void DestroyMultTable()
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
void Print_MultTable(FILE *filePtr) /* TW 9/19/93 - added 2 params to support view, save, & output */
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
          Print_AE(prod, filePtr); 	/* TW 9/22/93 - change prod[] to *prod */
          fprintf(filePtr, "\n\n");
        }
      }
    }   
  }
}
 
void Print_AE(const Alg_element &ae, FILE *filePtr) /* TW 9/19/93 - add 2 params to support view, save, & output */
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

static const unsigned short mult_file_version = 0xabef;

bool save_mult_table(const char *fn) {
    FILE *f = fopen(fn, "wb");
    if (!f) {
        printf("Unable to open %s for writing\n", fn);
        return false;
    }

//    typedef std::map<std::pair<Basis, Basis>, std::vector<std::pair<Basis, Scalar> > > mult_table_t;
//    typedef unsigned char Scalar;
//    typedef int Basis;

    fwrite(&mult_file_version, sizeof(mult_file_version), 1, f);

    const int ni = mult_table.size();
    fwrite(&ni, sizeof(ni), 1, f);

    for (const auto & i : mult_table) {
        auto bb = i.first;
        fwrite(&bb.first, sizeof(bb.first), 1, f);
        fwrite(&bb.second, sizeof(bb.second), 1, f);

        int nj = i.second.size();
        fwrite(&nj, sizeof(nj), 1, f);

        for (auto bs : i.second) {
            fwrite(&bs.first, sizeof(bs.first), 1, f);
            fwrite(&bs.second, sizeof(bs.second), 1, f);
        }
    }

    fclose(f);

    return true;
}

bool restore_mult_table(const char *fn) {
    FILE *f = fopen(fn, "rb");
    if (!f) {
        printf("Unable to open %s for reading\n", fn);
        return false;
    }

    mult_table.clear();

    unsigned short v;
    fread(&v, sizeof(v), 1, f);
    if(v != mult_file_version) {
        printf("Found incompatible file version while reading %s\n", fn);
        fclose(f);
        return false;
    }

    int ni;
    fread(&ni, sizeof(ni), 1, f);

    for (int i=0; i<ni; i++) {
        Basis b1, b2;
        fread(&b1, sizeof(b1), 1, f);
        fread(&b2, sizeof(b2), 1, f);
        pair<Basis, Basis> key(b1, b2);

        int nj;
        fread(&nj, sizeof(nj), 1, f);

        vector<pair<Basis, Scalar> > row(nj);

        for (int j=0; j<nj; j++) {
            Basis b;
            Scalar s;
            fread(&b, sizeof(b), 1, f);
            fread(&s, sizeof(s), 1, f);
            row[j] = make_pair(b, s);
        }

        mult_table.insert(make_pair(key, row));
    }

    fclose(f);

    return true;
}

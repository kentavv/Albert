#ifndef _MULT_TABLE_H_
#define _MULT_TABLE_H_

/*******************************************************************/
/***  FILE :        Mult_table.h                                 ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/***  MODIFICATION:  9/93 - Trent Whiteley                       ***/
/***                        changed Terms_list and terms from    ***/
/***                        arrays to pointers                   ***/
/*******************************************************************/

#include "Build_defs.h"
#include "Alg_elements.h"

#ifndef MTB_SIZE
#define MTB_SIZE 100
#endif

/* TW 9/22/93 - Terms_list changed from [] to * */
typedef struct {
    Scalar coef;
    Basis word;
} Term,*Terms_list,*Termptr,*Mt_block[MTB_SIZE][MTB_SIZE];


/* A block of Terms. */ 
/* Each of these point to next block of terms. */
/* Linear list of terms in abstract sense. */
/* Storage unit for building Mult.Table. */
typedef struct terms_block {
    Term *terms;		/* TW 9/22/93 - changed terms from array to ptr */
    struct terms_block *next;
} Terms_block;

int CreateMultTable(void);
int EnterProduct(Basis B1, Basis B2, Term Tl[]);
int Mult2basis(Basis B1, Basis B2, Scalar x, Alg_element *P);
int DestroyMultTable(void);
Term *Alloc_Terms_list(void);
int Print_MultTable(FILE *filePtr, int outputType);

#endif

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

#include <stdio.h>
#include <stdlib.h>

#include "Build_defs.h"
#include "Alg_elements.h"
#include "Mult_table.h"

#define  DEBUG_DESTROY_MT  0

/* Table of pointers to table of pointers to Term_lists. */
/*static Mt_block *Mt_block_index[MTB_INDEX_SIZE][MTB_INDEX_SIZE];*/
Mt_block ***Mt_block_index;

/* Pointer to first block of terms. */
/* Useful in freeing the Memory */
static Terms_block *First_terms_block;

/* Pointer to current block of terms being filled. */
static Terms_block *Cur_terms_block;

/* Offset within current block of terms being filled. */
/* There is free space for terms from this number onwards. */
static int Cur_offset;

/* TW 9/18/93 - line counter for view */
int lineCnt;


/*******************************************************************/
/* GLOBALS INITIALIZED:                                            */
/*     Mt_block_index -- Translation table.                        */ 
/*     First_terms_block                                           */
/*     Cur_terms_block -- Currently filled Terms_block.            */
/*     Cur_offset -- Position from where Cur_terms_block is free.  */
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Initialize the Translation Table.                           */
/*     Allocate memory for First_terms_block.                      */
/*     Initialize Cur_terms_block and Cur_offset.                  */
/* NOTE:                                                           */
/*     Called only once, at the time System initialization.        */
/*******************************************************************/ 
int CreateMultTable()
{
    int setMtBlock();		/* TW 9/23/93 */
    Terms_block *Alloc_Terms_block();

    int i,j;

/* There are Mt_blocks initially. */
    for (i=0;i<MTB_INDEX_SIZE;i++) 
       for (j=0;j<MTB_INDEX_SIZE;j++) 
/*          Mt_block_index[i][j] = NULL; TW 9/23/93 */
	  setMtBlock(i, j, NULL);	/* TW 9/23/93 */

    First_terms_block = Cur_terms_block = NULL;
 
    First_terms_block = Cur_terms_block = Alloc_Terms_block();
    
    assert_not_null(First_terms_block);

    Cur_offset = 0;

    return(OK);
}

    
/*******************************************************************/
/* GLOBALS MODIFIED:                                               */
/*     Mt_block_index -- If space is allocated for new Mt_block.   */ 
/*     Cur_terms_block -- If space is allocated for new Terms_block.*/ 
/*     Cur_offset -- Position from where Cur_terms_block is free.  */
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     B1,B2 -- Basis elements.                                    */
/*     Tl -- Terms_list.                                           */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Enter the product of Basis B1 and B2 in the Multiplication  */
/*     table as Terms_list.                                        */
/*******************************************************************/ 
int EnterProduct(B1,B2,Tl)
Basis B1;
Basis B2;
Term Tl[];
{
    Mt_block *getMtBlock();	/* TW 9/23/93 */
    int setMtBlock();		/* TW 9/23/93 */
    Mt_block *Alloc_Mt_block();
    Terms_block *Alloc_Terms_block();
    int TermsListLength();

    Terms_block  *new_terms_block;

    Mt_block *cur_mt_block;
    int mtbi_row,mtbi_col;
    int mtb_row,mtb_col;
    int tl_length; 
    int i;

/* Position in Mt_block_index table where pointer to rqd. Mt_block is stored. */
    mtbi_row = (B1 - 1)/MTB_SIZE; 
    mtbi_col = (B2 - 1)/MTB_SIZE; 

/* Position in Mt_block where pointer to Terms_list is stored. */
    mtb_row = (B1 - 1) % MTB_SIZE;
    mtb_col = (B2 - 1) % MTB_SIZE;

/* First reference this Mt_block. So allocate space. */
/*    if (Mt_block_index[mtbi_row][mtbi_col] == NULL)
        Mt_block_index[mtbi_row][mtbi_col] = Alloc_Mt_block(); TW 9/23/93 */
    if(getMtBlock(mtbi_row, mtbi_col) == NULL){		/* TW 9/23/93 */
      setMtBlock(mtbi_row, mtbi_col, Alloc_Mt_block());	/* TW 9/23/93 */
    }

/*    cur_mt_block = Mt_block_index[mtbi_row][mtbi_col]; TW 9/23/93 */
    cur_mt_block = getMtBlock(mtbi_row, mtbi_col);	/* TW 9/23/93 */

    assert_not_null(cur_mt_block);

    if ((*cur_mt_block)[mtb_row][mtb_col] != NULL)
        return(0); 
/* Find the number of terms that need to be copied. */
    tl_length = TermsListLength(Tl);

/* check number of cells left in the current terms block. */
    if ((DIMENSION_LIMIT - Cur_offset) < tl_length) { 
        new_terms_block = Alloc_Terms_block();
        assert_not_null(new_terms_block);
        Cur_terms_block->next = new_terms_block;
        Cur_terms_block = new_terms_block; 
        Cur_offset = 0;
    }

/* Store the address of Terms_list in Mt_block. */
    (*cur_mt_block)[mtb_row][mtb_col] = &(Cur_terms_block->terms[Cur_offset]);
        
/* Copy the Terms_list into Cur_terms_block. */
    for (i=0;i<tl_length;i++) {
        (Cur_terms_block->terms[Cur_offset]).coef = Tl[i].coef; 
        (Cur_terms_block->terms[Cur_offset]).word = Tl[i].word; 
        Cur_offset++;
    }
    return(OK);
}
        

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Tl -- Terms_list.                                           */
/* RETURNS:                                                        */
/*     Number of Terms in the Terms_list Tl.                       */
/*******************************************************************/ 
int TermsListLength(Tl)
Term Tl[];
{
    int length = 1;
    int i = 0;

    assert_not_null(Tl);

    while (Tl[i].coef != 0) {
        length++;
        i++;
    }
    return(length);
} 


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     B1,B2 -- Basis elements.                                    */
/* RETURNS:                                                        */
/*     Pointer to Terms_list, product of B1 and B2.                */ 
/*******************************************************************/ 
Term *RetrieveProduct(B1,B2)
Basis B1;
Basis B2;
{
    Mt_block *getMtBlock();	/* TW 9/23/93 */

    int mtbi_row,mtbi_col;
    int mtb_row,mtb_col;
    Mt_block *MTBlock;

/* Position in Mt_block_index table where pointer to rqd. Mt_block is stored. */

    mtbi_row = (B1 - 1)/MTB_SIZE; 
    mtbi_col = (B2 - 1)/MTB_SIZE; 

/* Position in Mt_block where pointer to Terms_list is stored. */
    mtb_row = (B1 - 1) % MTB_SIZE;
    mtb_col = (B2 - 1) % MTB_SIZE;

/*    if (Mt_block_index[mtbi_row][mtbi_col] == NULL) TW 9/23/93 */
    if(getMtBlock(mtbi_row, mtbi_col) == NULL)	/* TW 9/23/93 */
        return(NULL);

/*    return((*Mt_block_index[mtbi_row][mtbi_col])[mtb_row][mtb_col]); TW 9/23/93 */
    MTBlock = getMtBlock(mtbi_row, mtbi_col);	/* TW 9/23/93 */
    return((*MTBlock)[mtb_row][mtb_col]);	/* TW 9/23/93 */
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     B1,B2 -- Basis elements.                                    */
/* RETURNS:                                                        */
/*     Pointer to Terms_list, product of B1 and B2.                */ 
/*******************************************************************/ 
int Mult2basis(B1,B2,x,P)
Basis B1;
Basis B2;
Scalar x;
Alg_element *P;
{
    Scalar S_add();
    Scalar S_mul();

    Term *tl;
    Basis w;

    assert_not_null(P);

    tl = RetrieveProduct(B1,B2);
    if (tl == NULL)
        return(0);

    while (tl->coef != 0) {
        w = tl->word;
        P->basis_coef[w] = S_add(P->basis_coef[w],S_mul(x,tl->coef));
        tl++;
    }
    AssignFirst(P);
    AssignLast(P);
    return(OK);
}
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
int DestroyMultTable()
{
    Mt_block *getMtBlock();	/* TW 9/23/93 */
    int setMtBlock();		/* TW 9/23/93 */
    int FreeTermsBlocks();
    int TB_count = 1; /* Num. of Terms_block's allocated. For Stats. */
    int MT_count = 1; /* Num. of MT_block's allocated. For Stats. */

    int i,j;

    assert_not_null(Mt_block_index);	/* TW 9/27/93 - forgot to free this up */

    for (i=0;i<MTB_INDEX_SIZE;i++)
       for (j=0;j<MTB_INDEX_SIZE;j++) {
/*          if (Mt_block_index[i][j] != NULL) {
              free(Mt_block_index[i][j]);
              MT_count++;
              Mt_block_index[i][j] = NULL;
          } TW 9/23/93 */
          if(getMtBlock(i, j) != NULL) {	/* TW 9/23/93 */
              free(getMtBlock(i, j));		/*     |      */
              MT_count++;			/*     |      */
              setMtBlock(i, j, NULL);		/*     V      */
          }
       }

/*    for(i = 0; i < MTB_INDEX_SIZE; ++i){*//* TW 9/27/93 - forgot to free this up */
/*      assert_not_null(Mt_block_index[i]);
      free(Mt_block_index[i]);
    }

    free(Mt_block_index);*/		/* TW 9/27/93 - forgot to free this up */
   
    assert_not_null(First_terms_block);

    FreeTermsBlocks(First_terms_block,&TB_count);

#if DEBUG_DESTROY_MT
    i = DIMENSION_LIMIT + 4;
    printf("TB_count = %d Each TB size = %d\n",TB_count,i);
    i = MTB_SIZE * MTB_SIZE; 
    printf("MT_count = %d Each MT size = %d\n",MT_count,i);
#endif

    return(OK);
}

/*******************************************************************/
/* MODIFIES: None.                                                 */
/*     P -- Pointer to First_terms_block.                          */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Recursively Free the memory occupaid by all the Terms_blocks*/
/*******************************************************************/ 
int FreeTermsBlocks(P,X)
Terms_block *P;
int *X; /* For Stats. */
{
    assert_not_null(P);

    *X = *X + 1;

    if(P->next == NULL){
      if(P->terms != NULL){     /* TW 9/23/93 - forgot to free it */
        free(P->terms);
      }
      free(P);
    }
    else{
      FreeTermsBlocks(P->next,X);
      if(P->terms != NULL){     /* TW 9/23/93 - forgot to free it */
        free(P->terms);
      }
      free(P);
    }
    return(OK);
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *Q -- Terms_list.                                           */
/* REQUIRES:                                                       */
/*     P -- Pointer to Algabraic element.                          */ 
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Translate Algabraic element *P into Terms_list *Q.          */
/*******************************************************************/ 
int TransAETL(P,Q)
Alg_element *P;
Term *Q;
{
    int i;

    assert_not_null(P);
    assert_not_null(Q);

    for (i=P->first;i<=P->last;i++) {
       if (P->basis_coef[i] != 0) {
           Q->word = i;
           Q->coef = P->basis_coef[i];
           Q++;
       }
    }
    Q->word = 0;
    Q->coef = 0; 
    return(OK);
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *P -- Algabraic element.                                    */ 
/* REQUIRES:                                                       */
/*     Q -- Pointer to Terms_list.                                 */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Translate Terms_list *Q into Algabraic element *P.          */
/*******************************************************************/ 
int TransTLAE(P,Q)
Term *P;
Alg_element *Q;
{
    Mt_block *getMtBlock();	/* TW 9/23/93 */

    int i = 0;

    assert_not_null(P);
    assert_not_null(Q);

    ZeroOutAE(Q);

    while ((i<DIMENSION_LIMIT) && (P->coef != 0)) {
        Q->basis_coef[P->word] += P->coef;  
        P++;
        i++;
    }
    AssignFirst(Q);
    AssignLast(Q);

    return(OK);
}

int PrintMT()
{
    Mt_block *getMtBlock();	/* TW 9/23/93 */
    int i,j;

    for (i=0;i<MTB_INDEX_SIZE;i++)
        for (j=0;j<MTB_INDEX_SIZE;j++) {
            printf("Mtbi[%d][%d]\n",i,j);
/*            if (Mt_block_index[i][j] != NULL)
                PrintMTBlock(Mt_block_index[i][j]); TW 9/23/93 */
	    if(getMtBlock(i, j, NULL)){		/* TW 9/23/93 */
	      PrintMTBlock(getMtBlock(i, j));	/* TW 9/23/93 */
	    }
        }
    return(OK);
}


int PrintMTBlock(P)
Mt_block *P;
{
    int i,j;

    assert_not_null(P);

    for (i=0;i<MTB_SIZE;i++)
        for (j=0;j<MTB_SIZE;j++) {
            printf("Mtb[%d][%d]\n",i,j);
            if ((*P)[i][j] != NULL)
                PrintTL((*P)[i][j]);
        }
    return(OK);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     P -- Terms_list.                                            */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Print the Terms_list P.                                     */
/*******************************************************************/ 
int PrintTL(P)
Term *P;
{
    int i = 0;
    int j;
    int k = 0;

    assert_not_null(P);

    j = TermsListLength(P);

/*    printf("Terms List:\n");  */

    if (j <= 1)
        return(OK);

    for (i=1;i<j;i++) {
         printf(" + %d b[%d]",P->coef,P->word);
         P++;
         k = (++k)%7;
         if (k == 0)
             printf("\n");
    }
    printf("\n");
    return(OK);
}
           

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to newly allocated Mt_block.                        */
/* FUNCTION:                                                       */
/*     Allocate space for a new Mt_block and initialize it.        */
/*******************************************************************/ 
Mt_block *Alloc_Mt_block()
{
    char *Mymalloc();

    Mt_block *new_mt_block = NULL;
    int i,j;
    
    new_mt_block = ((Mt_block *) Mymalloc(sizeof(Mt_block)));

    assert_not_null(new_mt_block);

    for (i=0;i<MTB_SIZE;i++)
       for (j=0;j<MTB_SIZE;j++)
          (*new_mt_block)[i][j] = NULL;
    return(new_mt_block);
}


/* TW 9/22/93 - Terms_list change; added dynamic allocation */
/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to newly allocated Terms_list.                      */
/* FUNCTION:                                                       */
/*     Allocate space for a new Terms_list			   */
/*******************************************************************/
Term *Alloc_Terms_list()
{
    char *Mymalloc();

    Term *new_terms_list = NULL;

    new_terms_list = ((Term *) Mymalloc(sizeof(Term) * DIMENSION_LIMIT));
    assert_not_null(new_terms_list);

    return(new_terms_list);
}




/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to newly allocated Terms_block.                     */
/* FUNCTION:                                                       */
/*     Allocate space for a new Terms_block and initialize it.     */
/*******************************************************************/ 
Terms_block *Alloc_Terms_block()
{
    char *Mymalloc();

    Terms_block *new_terms_block = NULL;
    int i;

    new_terms_block = ((Terms_block *) Mymalloc(sizeof(Terms_block)));
    assert_not_null(new_terms_block);

    /* TW 9/22/93 - change of terms from array to ptr */
    new_terms_block->terms = ((Term *) Mymalloc(sizeof(Term) * DIMENSION_LIMIT));
    assert_not_null(new_terms_block->terms);

    for (i=0;i<DIMENSION_LIMIT;i++) {
        (new_terms_block->terms[i]).coef = 0; 
        (new_terms_block->terms[i]).word = 0; 
    }
    new_terms_block->next = NULL;

    return(new_terms_block);
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
int Print_MultTable(filePtr, outputType)
FILE *filePtr;	/* TW 9/19/93 - added 2 params to support view, save, & output */
int outputType;
{
  Alg_element *AllocAE();	/* TW 9/22/93 - change prod[] to *prod */
  int DestroyAE();              /* TW 9/23/93 - need to free up memory */

  int i, j, dim;
  Alg_element *prod = AllocAE();/* TW 9/22/93 - change prod[] to *prod */

  void more();	/* TW 9/19/93 */

  assert_not_null(prod);	/* TW 9/22/93 - change prod[] to *prod */

  lineCnt = 0;			/* TW 9/19/93 - support for view */
  dim = GetNextBasisTobeFilled();
  if(dim > 0){
    fprintf(filePtr, "\n");
    fprintf(filePtr, "Multiplication table: \n");
    lineCnt += 2;		/* TW 9/19/93 - support for view */
    InitAE(prod);
    for (i = 1; i <= dim; i++) {
      for (j = 1; j <= dim; j++) {
        ZeroOutAE(prod);	/* TW 9/22/93 - change prod[] to *prod */
        Mult2basis(i, j, 1, prod);	/* TW 9/22/93 - change prod[] to *prod */
        if (!IsZeroAE(prod)) {	/* TW 9/22/93 - change prod[] to *prod */
          /* PrintBasis(i); */
          fprintf(filePtr, "(b%d)*(b%d)\n",i,j);
	  if(outputType == 1){	/* TW 9/19/93 - support for view */
	    ++lineCnt;
	    more(&lineCnt);
	  }
          /* PrintBasis(j); */
          Print_AE(prod, filePtr, outputType); 	/* TW 9/22/93 - change prod[] to *prod */
          fprintf(filePtr, "\n");
	  if(outputType == 1){	/* TW 9/19/93 - support for view */
	    ++lineCnt;
	    more(&lineCnt);
	  }
          fprintf(filePtr, "\n");
	  if(outputType == 1){	/* TW 9/19/93 - support for view */
	    ++lineCnt;
	    more(&lineCnt);
	  }
        }
      }
    }   
  }
  DestroyAE(prod);      /* TW 9/23/93 - Shouldn't we free this up? */
}
 
int Print_AE(ae, filePtr, outputType)
Alg_element *ae;
FILE *filePtr;	/* TW 9/19/93 - add 2 params to support view, save, & output */
int outputType;
{
  void more();	/* TW 9/19/93 */

  int x,i;
  int trmcnt = 0;		/* How many terms have been printed */
  int lnecnt = 0;		/* How many have been printed on current line */
  for (i = ae->first; i <= ae ->last; i++) {
    if ( (ae->basis_coef)[i] != 0) {
      x = ae->basis_coef[i]; 
      if ((trmcnt > 0) && (trmcnt % 4 == 0)) { 		/* 4 items per line */
        fprintf(filePtr, "\n");	
	if(outputType == 1){	/* TW 9/19/93 - support for view */
	  ++lineCnt;
	  more(&lineCnt);
	}
        lnecnt = 0;
      }
      if (lnecnt == 0){
	fprintf(filePtr, "   %3d b%-4d",x,i);
      }
      else{
        fprintf(filePtr, "+  %3d b%-4d",x,i);
      }
      trmcnt++;
      lnecnt++;
    }
  }
}


/* TW 9/23/93 - added in order to dynamically allocate Mt_block_index */

Mt_block *getMtBlock(row, col)
int row;
int col;
{
  Mt_block **rowPtr = Mt_block_index[row];

  assert_not_null(rowPtr);
  return(rowPtr[col]);
}


int setMtBlock(row, col, val)
int row;
int col;
Mt_block *val;
{
  Mt_block **rowPtr = Mt_block_index[row];

  assert_not_null(rowPtr);
  rowPtr[col] = val;
}

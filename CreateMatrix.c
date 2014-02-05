/*******************************************************************/
/***  FILE :        CreateMatrix.c                               ***/
/***  AUTHOR:       David P Jacobs                               ***/
/***  PROGRAMMER:   Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990.                                    ***/
/***  MODIFIED:     David Lee (August 1992)                      ***/
/***                   Added routines SparseCreateTheMatrix()    ***/
/***                and SparseFillTheMatrix() for sparse method  ***/
/***                support.                                     ***/
/***                9/93 - Trent Whiteley                        ***/
/***                changed Pair_present from a two-dimensional  ***/
/***                array to a ptr and added calls to get and    ***/
/***                values within it                             ***/
/***                                                             ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      int CreateMatrix()                                     ***/
/***      int SparseCreateTheMatrix()                            ***/
/***  PRIVATE ROUTINES:                                          ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines dealing with creating    ***/
/***      a Matrix for the given list of equations. (i.e list of ***/
/***      Basis pairs.                                           ***/ 
/***      1) Initialize PairPresent bit Matrix to 0's.           ***/
/***      2) Compute 'N', the number of distinct Basis pairs     ***/
/***         present in the given list of equations.             ***/
/***      3) malloc ColtoBP array of 'N' pairs w.o coefficients. ***/  
/***      4) Fill the ColtoBP array with the distinct Bais pairs.***/
/***         Use the PairPresent bit Matrix while filling.       ***/
/***      5) Allocate space for the Matrix of equations.         ***/
/***         It's number of rows is equal to number of equations.***/
/***         And number of columns equal to 'N'.                 ***/
/***         ZeroOut the Matrix.                                 ***/
/***         For each equation, one row of the Matrix is filled. ***/
/***         For each Basis pair in the equation, scan the array ***/
/***         to find the column number. It is nothing but the    ***/
/***         position where we could find the required Basis pair***/
/***         Enter the Scalar corresponding to the Basis pair in ***/
/***         the equaition into the Matrix in the corresponding  ***/
/***         column.                                             ***/ 
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "CreateMatrix.h"
#include "Basis_table.h"
#include "Build_defs.h"
#include "Memory_routines.h"
#include "Scalar_arithmetic.h"
#include "Sparse_structs.h"
#include "Sparse_defs.h"
#include "SparseReduceMatrix.h"
#include "Type_table.h"

static void ZeroOutPairPresent(void);
static void FillPairPresent(void);
static void EnterPair(Basis i, Basis j);
static int CreateColtoBP(void);
static int AreBasisElements(Degree d);
static void Process(Degree d1, Degree d2, int *col_to_bp_index_ptr);
static int FillTheMatrix(void);
static int SparseFillTheMatrix(void);
#if 0
static void PrintPairPresent(void);
static void PrintColtoBP(void);
static void PrintTheMatrix(void);
#endif
static char getPairPresent(int row, int col);
static void setPairPresent(int row, int col, char val);

/* Added by DCL 8/92. These are variables used for gathering density stats */

extern short gather_density_flag;
extern long num_elements;
extern long max_num_elements;

char **Pair_present;
static char BIT_VECTOR[8]={'\200','\100','\040','\020','\010','\004','\002','\001'};

static int Num_unique_basis_pairs;
static int Num_equations;
static Unique_basis_pair_list ColtoBP;
static Eqn_list_node *First_eqn_list_node;
static MAT_PTR TheSparseMatrix;
static Matrix TheMatrix;
static Name N;



int CreateTheMatrix(Eqn_list_node *Eq_list, Matrix *Mptr, int *Rows, int *Cols, Unique_basis_pair_list *BPCptr, Name n)
{
    Num_unique_basis_pairs = 0;
    Num_equations = 0;
    ColtoBP = NULL;
    TheMatrix = NULL;
    N = n;

    First_eqn_list_node = Eq_list;

    if (Eq_list == NULL)
        return(OK);

    if (Eq_list->basis_pairs == NULL)
        return(OK);

    ZeroOutPairPresent();
    FillPairPresent();
/*
    PrintPairPresent();	
*/
    if (CreateColtoBP() != OK)
        return(0);
    if (FillTheMatrix() != OK)
        return(0);
    *Mptr = TheMatrix;
    *BPCptr = ColtoBP;
    *Rows = Num_equations;
    *Cols = Num_unique_basis_pairs; 
    return(OK);
}

/* Added by DCL (8/92). This is virtually identical to CreateTheMatrix()
   except that we call SparseFillTheMatrix() and copy the Matrix ptr to
   a pointer internal to this module to be altered and then copy it back
   before returning to SolveEquations() in Build.c */

int SparseCreateTheMatrix(Eqn_list_node *Eq_list, MAT_PTR *SMptr, int *Rows, int *Cols, Unique_basis_pair_list *BPCptr, Name n)
{
    Num_unique_basis_pairs = 0;
    Num_equations = 0;
    ColtoBP = NULL;
    TheSparseMatrix = NULL;
    N = n;

    First_eqn_list_node = Eq_list;

    if (Eq_list == NULL)
        return(OK);

    if (Eq_list->basis_pairs == NULL)
        return(OK);

    ZeroOutPairPresent();
    FillPairPresent();
/*
    PrintPairPresent();
*/
    if (CreateColtoBP() != OK)
        return(0);
    if (SparseFillTheMatrix() != OK)
        return(0);

    /* pass locally derived values back to the calling routine */

    *SMptr= TheSparseMatrix;
    *BPCptr = ColtoBP;
    *Rows = Num_equations;
    *Cols = Num_unique_basis_pairs; 
    return(OK);
}


void DestroyBPtoCol(void)
{
    assert_not_null_nv(ColtoBP);
    free(ColtoBP);
}


void DestroyTheMatrix(void)
{
    assert_not_null_nv(TheMatrix);
    free(TheMatrix);
}


void ZeroOutPairPresent(void)
{
    int i,j;

    for (i=0;i<DIMENSION_LIMIT;i++)
        for (j=0;j<PP_COL_SIZE;j++)
/*            Pair_present[i][j] = 0;*/
	    setPairPresent(i, j, 0);	/* 9/93 - TW - new Pair_present routine */
}


void FillPairPresent(void)
{
    Eqn_list_node *temp;
    int i;

    temp = First_eqn_list_node;
    while (temp != NULL) {
        i = 0;
        assert_not_null_nv(temp->basis_pairs);
        while (temp->basis_pairs[i].coef != 0) {
            EnterPair(temp->basis_pairs[i].left_basis,
                      temp->basis_pairs[i].right_basis);
            i++;
        }
        Num_equations++;
        temp = temp->next;
    }
}
                

void EnterPair(Basis i, Basis j)
{
    int row_byte,col_byte,col_bit;

    row_byte = i-1;
    col_byte = (j-1)/8;
    col_bit = (j-1)%8;

/*    if (!(Pair_present[row_byte][col_byte] & BIT_VECTOR[col_bit])) {
        Pair_present[row_byte][col_byte] |= BIT_VECTOR[col_bit];*/
    if(!(getPairPresent(row_byte, col_byte) & BIT_VECTOR[col_bit])){	/* 9/93 - TW - new Pair_present routines */
	setPairPresent(row_byte, col_byte, getPairPresent(row_byte, col_byte) | BIT_VECTOR[col_bit]);
        Num_unique_basis_pairs++;
    }
}



int CreateColtoBP(void)
{
    int col_to_bp_index = 0;

    int i;
    Degree d;

    if (Num_unique_basis_pairs == 0)
        return(OK);

    ColtoBP = (Unique_basis_pair_list) (Mymalloc(Num_unique_basis_pairs *
                                               sizeof(Unique_basis_pair)));
    assert_not_null(ColtoBP);

    d = GetDegreeName(N);
     
    for (i=1;i<d;i++)
        /*  	This check added 5/94   (DPJ)    */
        if ( AreBasisElements(i) && AreBasisElements(d-i) ) 
        {
            Process(i,d-i,&col_to_bp_index);
        }
    return(OK);
}

/*
5/94 (DPJ)
Returns true if there are basis elements at degree d.
Note: when the algebra is nilpotent it is possible
that no new basis elements were entered at degree d.
*/
int AreBasisElements(Degree d)
{
   if (BasisStart(d) == 0) 
       return FALSE;
   else
       return TRUE;
}


void Process(Degree d1, Degree d2, int *col_to_bp_index_ptr)
{
    Basis b1,b2,b3,b4;
    Basis row,col;
    int col_bit;
    Basis i,j;

    b1 = BasisStart(d1);
    b2 = BasisEnd(d1);
    b3 = BasisStart(d2);
    b4 = BasisEnd(d2);

    for (i=b1;i<=b2;i++) {
        for (j=b3;j<=b4;j++) {
            row = i - 1;
            col = (j-1)/8;
            col_bit = (j-1)%8;
/*            if (Pair_present[row][col] & BIT_VECTOR[col_bit]) {*/
	    if(getPairPresent(row, col) & BIT_VECTOR[col_bit]){	/* 9/93 - TW - new Pair_present routine */
                ColtoBP[*col_to_bp_index_ptr].left_basis = i; 
                ColtoBP[*col_to_bp_index_ptr].right_basis = j; 
                (*col_to_bp_index_ptr)++;
            }
        }
    }
}
            

int FillTheMatrix(void)
{
    Eqn_list_node *temp;
    int i,j;
    int eq_number = 0;
    int col;
    int thematrix_index;
    Scalar x;

    if ((Num_unique_basis_pairs == 0) || (Num_equations == 0))
        return(OK);
    TheMatrix = (Scalar *) (Mymalloc(Num_equations*Num_unique_basis_pairs
                                   *sizeof(Scalar)));
    assert_not_null(TheMatrix);
    for (i=0;i<Num_equations;i++)
        for (j=0;j<Num_unique_basis_pairs;j++) 
            TheMatrix[i * Num_unique_basis_pairs + j] = 0;

    temp = First_eqn_list_node;
    while (temp != NULL) {
        i = 0;
        if (temp->basis_pairs == NULL)
			{
            return(OK);
			}
        while (temp->basis_pairs[i].coef != 0) {
            col = GetCol(temp->basis_pairs[i].left_basis,
                         temp->basis_pairs[i].right_basis);
            thematrix_index = eq_number * Num_unique_basis_pairs + col;
            x = TheMatrix[thematrix_index];
            TheMatrix[thematrix_index] = S_add(x, temp->basis_pairs[i].coef);

            /* gather density statistics here...DCL 8/21/92 */
            if (gather_density_flag) 
            {
               if ((x== S_zero()) && (TheMatrix[thematrix_index] != S_zero()))
               {
                  num_elements++;
               }   
               if ((x != S_zero()) && (TheMatrix[thematrix_index] == S_zero()))
               {
                  num_elements--;
               }
	   }
            i++;
        }
        eq_number++;
        temp = temp->next;
    }
    return(OK);
}

int SparseFillTheMatrix(void)
{
    Eqn_list_node *temp;
    int i;
    int eq_number = 0;
    int col;
	 
    Scalar Tmp_S_Sum;
    NODE_PTR Prev_Ptr;
    NODE_PTR Look_Ahead_Ptr;

    if ((Num_unique_basis_pairs == 0) || (Num_equations == 0))
        return(OK);
	
    /* Allocate the row pointers */

    TheSparseMatrix = (MAT_PTR) (Mymalloc(Num_equations*sizeof(NODE_PTR)));


    assert_not_null(TheSparseMatrix);

    /* initialize the pointers */

    for (i=0;i < Num_equations;i++)
 	TheSparseMatrix[i]=NULL;


    temp = First_eqn_list_node;
    while (temp != NULL) 
    {
       i = 0;
       if (temp->basis_pairs == NULL)
           return(OK);
       while (temp->basis_pairs[i].coef != 0) 
       {
            col = GetCol(temp->basis_pairs[i].left_basis,
                         temp->basis_pairs[i].right_basis);
            Tmp_S_Sum = S_zero();


            /* since using a singly linked list we need ptr from the node
               before this row,col value which is what Locate_Node will
               return. Locate_Node will try to return the one just before
               the column node that we want */ 

	    Prev_Ptr = (NODE_PTR) Locate_Node(TheSparseMatrix,eq_number,col);

            /* if there is not one and the row is not empty this row,col
               value must be the first one in the row */

            if ((Prev_Ptr == NULL) && (!Row_empty(TheSparseMatrix,eq_number)))
            {
		Look_Ahead_Ptr=TheSparseMatrix[eq_number];
            }	
            else
            {
                if (Row_empty(TheSparseMatrix,eq_number))
                {
                   /* the row is empty */

                   Look_Ahead_Ptr=NULL;
		}
		else
		{
                   /* There is node after this one so we know that is
                      that we want to work with */

                   if (Prev_Ptr->Next_Node!=NULL)
                   {		
			Look_Ahead_Ptr=Prev_Ptr->Next_Node;
                   }
                   else
                   {
                   /* there is not a node after this one so this is
                      the one that we want */

			Look_Ahead_Ptr=Prev_Ptr;
                   }
		}
           }

           /* Look_Ahead Ptr will only be null if there are NO nodes
              in the row */

           if (Look_Ahead_Ptr != NULL)
           {
             
              if (Look_Ahead_Ptr->column != col)
	      {	
              /* Here we should add a node since there is no node in 
                 the row with the column value we are looking for */


  	 	Tmp_S_Sum=S_add(S_zero(),temp->basis_pairs[i].coef);
                if (Tmp_S_Sum != S_zero())
                {
                   Insert_Element(TheSparseMatrix,eq_number,Tmp_S_Sum, 
                                col,Prev_Ptr);

                /* if the flag is set increment # of elements for stat purpose*/

                   if (gather_density_flag)
                      num_elements++;
                }
	      }

	      else
              {
              /* There is a node here with the same column value we are
                 looking for so add the new value to this one */

	  	Tmp_S_Sum=S_add(Look_Ahead_Ptr->element, 
                                  temp->basis_pairs[i].coef);

		if (Tmp_S_Sum==S_zero())
		{
                /* If the result is zero then we will want to delete
                   this node */

                   Delete_Element(TheSparseMatrix,eq_number,Prev_Ptr);

                   /* gather statistical information */

                   if (gather_density_flag)
                      num_elements--;
		}
     
		else
		{
                /* the result was nonzero and we just change the node element
                   field to the result */

                    Change_Element(Look_Ahead_Ptr,Tmp_S_Sum);
		}
	      }
	   }

           /* We know that there were no nonzero elements in the row
              so lets add one */ 
           else
	   {
	      Tmp_S_Sum=S_add(S_zero(),temp->basis_pairs[i].coef);


              /* don't even add one if the result is zero*/

              if (Tmp_S_Sum != S_zero())
              Insert_Element(TheSparseMatrix,eq_number,Tmp_S_Sum,col,Prev_Ptr);

              /* gather statistics */

              if (gather_density_flag)
               num_elements++;
           }	
           i++;
        }
        eq_number++;
        temp = temp->next;
    }
    return(OK);
}


int GetCol(Basis Left_basis, Basis Right_basis)
{
    int low,high,middle;

    if ((Num_unique_basis_pairs == 0) || (Num_equations == 0))
        return(-1);
    low = 0;
    high = Num_unique_basis_pairs - 1;
    while (low <= high) {
        middle = (low + high)/2; 
        if (Left_basis < ColtoBP[middle].left_basis)
            high = middle - 1;
        else
            if (Left_basis > ColtoBP[middle].left_basis)
                low = middle + 1;
        else {
            if (Right_basis < ColtoBP[middle].right_basis)
                high = middle - 1;
            else
                if (Right_basis > ColtoBP[middle].right_basis)
                    low = middle + 1;
            else
                return(middle);
        }
    }
    return(-1);
}


#if 0
void PrintPairPresent(void)
{
    int i,j,k;

    assert_not_null_nv(Pair_present);

    for(i=0;i<DIMENSION_LIMIT;i++){
        for(j=0;j<PP_COL_SIZE;j++){
            for(k=0;k<8;k++){
/*                if (Pair_present[i][j] & BIT_VECTOR[k]) */
		if(getPairPresent(i, j) & BIT_VECTOR[k]){	/* 9/93 - TW - new Pair_present routine */
                    printf("1");
		}
                else{
                    printf("0");
		}
	    }
        }
        printf("\n");
    }
}


void PrintColtoBP(void)
{
    int i,j=0;

    assert_not_null_nv(ColtoBP);

    printf("The ColtoBP is : \n");

    for (i=0;i<Num_unique_basis_pairs;i++) {
        printf("b[%d]b[%d] ",ColtoBP[i].left_basis,ColtoBP[i].right_basis);
        j = (j+1)%7;
        if (j == 0)
            printf("\n");
    }
    printf("\n");
}


void PrintTheMatrix(void)
{
    int i,j,k=0;
    int thematrix_index;

    assert_not_null_nv(TheMatrix);

    printf("The Matrix to be solved is : \n");

    for (i=0;i<Num_equations;i++) {
        for (j=0;j<Num_unique_basis_pairs;j++) {
            thematrix_index = i * Num_unique_basis_pairs + j;
            printf(" %3d",TheMatrix[thematrix_index]);
            k = (k + 1)%25;
            if (k == 0)
                printf("\n");
        }
        k = 0;
        printf("\n");
    }
    printf("\n");
}
#endif


/****************************************************************/
/* MODIFIES: None.                                              */
/* REQUIRES: row - the desired row in Pair_present              */
/*           col - the desired column in Pair_present           */
/* RETURNS:  the value of Pair_present[row][col]                */
/* FUNCTION: return the desired value from Pair_present         */
/****************************************************************/
char getPairPresent(int row, int col)
{
  char *rowPtr = Pair_present[row];

  return(rowPtr[col]);
}


/****************************************************************/
/* MODIFIES: Pair_present[row][col]                             */
/* REQUIRES: row - the desired row in Pair_present              */
/*           col - the desired column in Pair_present           */
/*           val - the value to be placed in Pair_present[r][c] */
/* RETURNS:  None                                               */
/* FUNCTION: set the desired value within Pair_present          */
/****************************************************************/
void setPairPresent(int row, int col, char val)
{
  char *rowPtr = Pair_present[row];

  rowPtr[col] = val;
}


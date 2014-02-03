/*******************************************************************/
/***  FILE :        ReduceMatrix.c                               ***/
/***  AUTHOR:       David P Jacobs                               ***/
/***  PROGRAMMER:   Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990.                                    ***/
/***  MODIFIED:     David Lee (Aug 1992).                        ***/
/***                     - Added code to gather statistics.      ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      int ReduceMatrix()                                     ***/
/***  PRIVATE ROUTINES:                                          ***/
/***  MODULE DESCRIPTION:                                        ***/
/*******************************************************************/

#include <stdio.h>

#include "ReduceMatrix.h"
#include "Build_defs.h"
#include "CreateMatrix.h"

    void S_init();
    Scalar S_one();
    Scalar S_inv();
    Scalar S_minus();
    Scalar S_zero();
    Scalar S_add();
    Scalar S_mul();

static void Interchange(int Row1, int Row2);
static void Knockout(int Row, int Col);
static void MultRow(int Row, Scalar Factor);
static void AddRow(Scalar Factor, int Row1, int Row2);
//static int GetRank(void);
static void PrintTheRMatrix(void);
//static void CreateRandomMatrix(void);


/*********************
These are for density measurement on last matrix. 
*********************/
extern short int gather_density_flag;
extern long unsigned int num_elements;
extern long unsigned int max_num_elements;

static Matrix TheMatrix;
static int Num_rows;
static int Num_cols;


int ReduceTheMatrix(Matrix Mptr, int Rows, int Cols, int *Rank)
{
    int i,j;
    int nextstairrow = 0;					/* next row with stair step 1 */

    TheMatrix = Mptr; 
    Num_rows = Rows;
    Num_cols = Cols;

    if ((Rows == 0) || (Cols == 0))
        return(OK);

    assert_not_null(Mptr);

    for (i=0;i<Num_cols;i++) {
        for (j=nextstairrow;j<Num_rows;j++) {
             if (TheMatrix[j*Num_cols + i] != S_zero())
                 break;
        }
        if (j<Num_rows) {
            Interchange(nextstairrow,j);
            Knockout(nextstairrow,i);
			   nextstairrow++;
        }
    }

    *Rank = nextstairrow;

    return(OK);
}


void Interchange(int Row1, int Row2)
{
    int j;
    int row1_start,row2_start;
    Scalar temp;
   
    row1_start = Row1 * Num_cols; 
    row2_start = Row2 * Num_cols; 

    for (j=0;j<Num_cols;j++) {
        temp = TheMatrix[row1_start + j];
        TheMatrix[row1_start + j] = TheMatrix[row2_start + j];
        TheMatrix[row2_start + j] = temp;
    }
}


void Knockout(int Row, int Col)
{
    int j;
    Scalar x;
    Scalar one = S_one();

    if ((x = TheMatrix[Row*Num_cols + Col]) != one)
        MultRow(Row,S_inv(x));
    for (j=0;j<Row;j++)
        AddRow(S_minus(TheMatrix[j*Num_cols + Col]),Row,j);
    for (j=Row+1;j<Num_rows;j++)
        AddRow(S_minus(TheMatrix[j*Num_cols + Col]),Row,j);
}


void MultRow(int Row, Scalar Factor)
{
    int j;
    int row_start = Row * Num_cols;

    for (j=0;j<Num_cols;j++)
    {
        TheMatrix[row_start + j] = S_mul(TheMatrix[row_start + j],Factor);
    } 
} 


void AddRow(Scalar Factor, int Row1, int Row2)
{
    Scalar before;
    Scalar after;

    int j;
    int row1_start,row2_start;
   
    if (Factor == S_zero())
        return;

    row1_start = Row1 * Num_cols; 
    row2_start = Row2 * Num_cols; 

    for (j=0;j<Num_cols;j++)
	 {
        before = TheMatrix[row2_start+j];
        TheMatrix[row2_start + j] = S_add(TheMatrix[row2_start + j],
                                       S_mul(Factor,TheMatrix[row1_start + j])); 
        if (gather_density_flag)
        {
           after = TheMatrix[row2_start+j];
           if ((before==S_zero()) && (after != S_zero()))
           {
              num_elements++;
              if (num_elements > max_num_elements)
              {
                 max_num_elements=num_elements;
              }
           }
           if ((before!=S_zero()) && (after == S_zero()))
              num_elements--;
       }
	 }
		
		
}
    

/*
int GetRank(void)
{
    int i,j;
    int rank = 0;

    for (i=0;i<Num_rows;i++) {
        for (j=0;j<Num_cols;j++) {
            if (TheMatrix[i*Num_cols + j] != S_zero()) {
                rank++;
                break;
            }
        }
    }
    return(rank);
}

*/

void PrintTheRMatrix(void)
{
    int i,j,k=0;

    printf("The Reduced Matrix is : \n");
    for (i=0;i<Num_rows;i++) {
        for (j=0;j<Num_cols;j++) {
            printf(" %3d",TheMatrix[i*Num_cols + j]);
            k = (k + 1)%25;
            if (k == 0)
                printf("\n");
        }
        k = 0;
        printf("\n");
    }
    printf("\n");
}

/*
int main(void)
{
    S_init();

    CreateRandomMatrix();
    ReduceTheMatrix();
    PrintTheRMatrix();
    printf("Num_rows = %d Rank = %d\n",Num_rows,GetRank());

    return 0;
}

void CreateRandomMatrix(void)
{
    int i,j,k;
    int Num_cells;

    Num_cols = Num_rows = rand() % 10 + 10;
    Num_cells = Num_rows * Num_cols;
    TheMatrix = (Scalar *) (Mymalloc(Num_cells * sizeof(Scalar))); 
    assert_not_null(TheMatrix);
    for (i=0;i<Num_rows;i++)
        for (j=0;j<Num_rows;j++)
            TheMatrix[i*Num_rows + j] = 0;
    for (i=0;i<(Num_rows-4);i++)
            TheMatrix[i*Num_rows +i] = 1;
    PrintTheRMatrix();
    printf("Before reduction: Rank =  %d\n",GetRank());
    for (i=0;i<25;i++) {
        Interchange(rand()%Num_rows,rand()%Num_rows);
        MultRow(rand()%Num_rows,rand()%25);
        AddRow(rand()%25,rand()%Num_rows,rand()%Num_rows);
    }
}

*/

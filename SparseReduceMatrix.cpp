/******************************************************************/
/***  FILE :          SparseReduceMatrix.c                      ***/
/***  PROGRAMMER:     David Lee                                 ***/
/***  DATE WRITTEN:   April-August 1992.                        ***/
/***  PUBLIC ROUTINES:                                          ***/
/***                  SparseReduceMatrix()                      ***/
/***                  Get_Matrix_Element()                      ***/
/***                  Insert_Element()                          ***/
/***                  Delete_Element()                          ***/
/***                  Change_Element()                          ***/
/***                  Locate_Node()                             ***/
/***  PRIVATE ROUTINES:                                         ***/
/***                  SparseMultRow()                           ***/
/***                  SparseAddRow()                            ***/
/***                  SparseKnockOut()                          ***/
/***                  SparseInterchange()                       ***/
/***                  Insert_Node()                             ***/
/***                  Delete_Node()                             ***/
/***                  Change_Node()                             ***/
/***                  Row_Empty()                               ***/
/***  MODULE DESCRIPTION:                                       ***/
/***                   This module  reduces the sparse matrix   ***/
/***                   in row canonical form. This code is      ***/
/***                   similar to the code in ReduceMatrix.c    ***/
/******************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <omp.h>

#include "SparseReduceMatrix.h"
#include "Build_defs.h"
#include "Scalar_arithmetic.h"
#include "Sparse_structs.h"
#include "Sparse_defs.h"
#include "node_mgt.h"

static void SparseMultRow(int Row, Scalar Factor);
static void SparseAddRow(Scalar Factor, int Row1, int Row2);
static void SparseInterchange(int Row1, int Row2);
static void SparseKnockOut(int row, int col, int nRows);
#if 0
static void Print_Matrix(MAT_PTR Sparse_Matrix, int r, int c);
static void Print_Rows(int Row1, int Row2, int nCols);
static void Print_SLList(Node *SLHead_Ptr);
static void Print_Node(NODE_PTR Prt_Node);
#endif
static void Insert_Node(MAT_PTR Sparse_Matrix, int matrow, NODE_PTR New_Node, NODE_PTR Prev_Ptr);
static void Delete_Node(MAT_PTR Sparse_Matrix, int RowId, NODE_PTR Prev_Ptr);
static NODE_PTR Locate_Element(NODE_PTR Tmp_Ptr, int row, int col);

/* These external variables are for measuring density of last generated matrix*/

extern int gather_density_flag;
extern long num_elements;
extern long max_num_elements;

static MAT_PTR Matrix_Base_Ptr = NULL;


int SparseReduceMatrix(MAT_PTR *Matrix_BPtr, int nRows, int nCols, int *Rank)
{
    int i,j;
    int nextstairrow = 0;
    Scalar x;

    Matrix_Base_Ptr = *Matrix_BPtr;

    if(nRows == 0 || nCols == 0)
    {
        return(OK);
    }

    assert_not_null(Matrix_BPtr);

    /* Search for the rightmost nonzero element */
    /* Dependent on the current stairrow */

    for (i=0;i<nCols;i++)
    {
        for (j=nextstairrow;j < nRows;j++)
        {
            if ((x=Get_Matrix_Element(Matrix_Base_Ptr,j,i)) != S_zero())
            {
                break;
            }
        }
   
        /* When found interchange and then try  to knockout any nonzero
           elements in the same column */

        if (j < nRows)
        {
           SparseInterchange(nextstairrow,j);
           SparseKnockOut(nextstairrow,i, nRows);
           nextstairrow++;
        }
    }
    *Rank=nextstairrow;
    *Matrix_BPtr = Matrix_Base_Ptr;

    return(OK);
}


void SparseMultRow(int Row, Scalar Factor)
{
   /* Get the first node in the row */
   /* Step thru row ... multiplying each element by the factor */

   for(NODE_PTR PMR_Ptr = Matrix_Base_Ptr[Row]; PMR_Ptr; PMR_Ptr=PMR_Ptr->Next_Node) {
      PMR_Ptr->element=S_mul(PMR_Ptr->element,Factor);
   }
}

/*********************************************************************/
/* Three things can happen with the SparseAddRow routine ....
   First there is a target row and a row which is multiplied by a factor
   and added to the target row. 
   1. The result is nonzero and there is no column in the target row so add 
      a new node.
   2. The result is nonzero and there is a column in the target row so change
      the value in the node.
   3. The result is zero and there is a column in the target row so delete
      the node.
*/
/*********************************************************************/
void SparseAddRow(Scalar Factor, int Row1, int Row2)
{
    /* check for zero factor */

   if (Factor == S_zero())
   {
      return;
   }

   /* get the beginning of the two rows to work with */

   NODE_PTR Row1_Ptr = Matrix_Base_Ptr[Row1];
   NODE_PTR Temp_Ptr = Matrix_Base_Ptr[Row2];

    Scalar TmpRow2_S_Sum;
    NODE_PTR Prev_Ptr;
    NODE_PTR Look_Ahead_Ptr;


  /* Process the linked list representing the row to be multiplied and
     to be added */

   while (Row1_Ptr)
   {
         /* go ahead and perform the scalar multiplication */

      Scalar TmpRow1_S_Product = S_mul(Factor,Row1_Ptr->element);

         /* Try to get the previous node to the one that we really want 
            which is the one that would match the column value of the
            one that we wish to add */

      
      Prev_Ptr = Locate_Element(Temp_Ptr,Row2,Row1_Ptr->column);
      Temp_Ptr = Prev_Ptr;
  
     
         /* if there is no previous and the row is not empty then 
            we want to work on the first node in the row */

      if (!Prev_Ptr && !Row_empty(Matrix_Base_Ptr,Row2))
      {
          Look_Ahead_Ptr=Matrix_Base_Ptr[Row2];
      }
      else
      {
         if (Row_empty(Matrix_Base_Ptr,Row2))
         {
             /* Row is empty */

             Look_Ahead_Ptr=NULL;
         }
         else
         {
        /* Row is not empty so we must be before the node we want to
           work with so if there is a next node that is the one we want*/

            if (Prev_Ptr->Next_Node)
            {
                Look_Ahead_Ptr=Prev_Ptr->Next_Node;
            }

            /* there is not another one in the row so we want to work
               with this one */

            else
            {
                Look_Ahead_Ptr=Prev_Ptr;
            }
        }
    }


    if (Look_Ahead_Ptr)
    {
    /* Row wasn't empty */

       if (Look_Ahead_Ptr->column != Row1_Ptr->column)
       {
       /* The row doesn't have a node representing the actual 
          column that we want so lets add one */

          TmpRow2_S_Sum = TmpRow1_S_Product;
          Insert_Element(Matrix_Base_Ptr,Row2,TmpRow2_S_Sum,
          Row1_Ptr->column,Prev_Ptr);
     
          /* gather statistics */

          if (gather_density_flag)
          {
            num_elements++;
            if (num_elements > max_num_elements)
               max_num_elements = num_elements;
          }
       }
       else
       {
          /* the column we want is there */

          TmpRow2_S_Sum = S_add(Look_Ahead_Ptr->element,TmpRow1_S_Product);
          if (TmpRow2_S_Sum == S_zero())
          {
          /* the result is zero so we must delete it */

             Delete_Element(Matrix_Base_Ptr,Row2,Prev_Ptr);

             /* statistics */

             if (gather_density_flag)
               num_elements--;
          }
          else
          {
          /* the result is not zero and column is present so change value */

             Change_Element(Look_Ahead_Ptr,TmpRow2_S_Sum);
          }
       }
      }

      else
      {
      /* We know the row is empty and we will add a node if the
         result is nonzero */

          TmpRow2_S_Sum = TmpRow1_S_Product;
       
          /* the result is nonzero so insert node */

          Insert_Element(Matrix_Base_Ptr,Row2,TmpRow2_S_Sum,
                                         Row1_Ptr->column,Prev_Ptr);

          /*statistics */

          if (gather_density_flag)
          {
             num_elements++;
             if (num_elements > max_num_elements)
             max_num_elements = num_elements;
          }
      }
                        
      Row1_Ptr=Row1_Ptr->Next_Node;
   }
}


void SparseInterchange(int Row1, int Row2)
{
        /* switch the two row pointers */
        NODE_PTR Temp_Row_Ptr = Matrix_Base_Ptr[Row1];
        Matrix_Base_Ptr[Row1] = Matrix_Base_Ptr[Row2];
        Matrix_Base_Ptr[Row2] = Temp_Row_Ptr;
}


void SparseKnockOut(int row, int col, int nRows)
{
    Scalar x;

    int j;
    /*int i;*/
    Scalar one = S_one();

    if ((x=Get_Matrix_Element(Matrix_Base_Ptr,row,col)) !=one)
    {
    /* if the rightmost element in the current row is not one then multiply*/

        SparseMultRow(row,S_inv(x));
    }

    /* try to knockout elements in column in the rows above */ 

#pragma omp parallel for schedule(dynamic, 10)
    for (j=0;j < nRows;j++)
    {
      if(j != row) {
        SparseAddRow(S_minus(Get_Matrix_Element(Matrix_Base_Ptr, j, col)), row, j);
      }
    }
}

#if 0
void Print_Matrix(MAT_PTR Sparse_Matrix, int r, int c)
{
    int i,row,col;
    Node *Row_Head_Ptr,*Row_Element_Ptr;

    if (Sparse_Matrix==NULL)
    {
       return; 
    }
    for (row=0;row < r;row++)
    {
        Row_Head_Ptr = Sparse_Matrix[row];
        Row_Element_Ptr = Row_Head_Ptr;
        if (Row_Element_Ptr == NULL)
        {
            printf("EMPTY ROW %d\n",row);
        }
        else
        {
            while (Row_Element_Ptr !=NULL)
            {
                printf("%4d",Row_Element_Ptr->element);
                Row_Element_Ptr = Row_Element_Ptr->Next_Node;
            }
            printf("\n");
        }
    }
    printf("\n");
    for (row=0;row < r;row++)
    {
        Row_Head_Ptr = Sparse_Matrix[row];
        Row_Element_Ptr = Row_Head_Ptr;

        for (col=0;(col < c);col++)
        {
            if (Row_Element_Ptr != NULL)
            {
                if (Row_Element_Ptr->column != col)
                {
                    printf("   0");
                }
                else
                {
                    printf("%4d",Row_Element_Ptr->element);
                    if (Row_Element_Ptr->Next_Node != NULL)
                    Row_Element_Ptr = Row_Element_Ptr->Next_Node;
                }
            }
            else
            {
                printf("   0");                 
            }
        }
        printf("\n");
    }
    printf("\n");
}


void Print_Rows(int Row1, int Row2, int nCols)
{
    int i,row,col;
    NODE_PTR Row1_Ptr;
    NODE_PTR Row2_Ptr;

    Row1_Ptr = Matrix_Base_Ptr[Row1];
    Row2_Ptr = Matrix_Base_Ptr[Row2];
    for (col=0;(col < nCols);col++)
    {
        if (Row1_Ptr != NULL)
        {
            if (Row1_Ptr->column != col)
            {
                printf("   0");
            }
            else
            {
                printf("%4d",Row1_Ptr->element);
                if (Row1_Ptr->Next_Node != NULL)
                Row1_Ptr = Row1_Ptr->Next_Node;
            }
        }
        else
        {
            printf("   0");                 
        }
    }
    printf("\n");
    for (col=0;(col < nCols);col++)
    {
        if (Row2_Ptr != NULL)
        {
            if (Row2_Ptr->column != col)
            {
                printf("   0");
            }
            else
            {
                printf("%4d",Row2_Ptr->element);
                if (Row2_Ptr->Next_Node != NULL)
                        Row2_Ptr = Row2_Ptr->Next_Node;
            }
        }
        else
        {
            printf("   0");                 
        }
    }
    printf("\n");
}
#endif

Scalar Get_Matrix_Element(MAT_PTR Sparse_Matrix, int i, int j)
{
    /* either return the element at location i,j or return a zero */
    NODE_PTR Tmp_Ptr = Sparse_Matrix[i];
        while ((Tmp_Ptr != NULL) && (Tmp_Ptr->column <= j))
        {
            if (Tmp_Ptr->column == j)
            {       
                return(Tmp_Ptr->element);
            }
            Tmp_Ptr=Tmp_Ptr->Next_Node;
        }
    return(S_zero());
}


#if 0
void Print_SLList(Node *SLHead_Ptr)
{
    Node *Prt_Ptr;

    Prt_Ptr = SLHead_Ptr;

    printf("\nColumn :");
    while (Prt_Ptr != NULL)
    {
        printf(" %3d",Prt_Ptr->column);
        Prt_Ptr = Prt_Ptr->Next_Node;
    }

    Prt_Ptr = SLHead_Ptr;

    printf("\n");
    printf("Element:");
    while (Prt_Ptr != NULL)
    {
        printf(" %3d",Prt_Ptr->element);
        Prt_Ptr = Prt_Ptr->Next_Node;
    }
    printf("\n");
    printf("\n");
}
#endif

void Insert_Element(MAT_PTR Sparse_Matrix, int matrow, Scalar element, int column, NODE_PTR Prev_Ptr)
{
    NODE_PTR New_Node;
        
    /* get a new node structure from the memory management system */

#pragma omp critical(record)
{
    New_Node =(NODE_PTR) GetRecord();
}

    /* set the values of the structure */

    New_Node->element=element;
    New_Node->column=column;

    /* insert it into the correct spot in the matrix */

    Insert_Node(Sparse_Matrix,matrow,New_Node,Prev_Ptr);
}

void Delete_Element(MAT_PTR Sparse_Matrix, int RowId, NODE_PTR Prev_Ptr)
{
    /* Call primitive to delete element */

    Delete_Node(Sparse_Matrix,RowId,Prev_Ptr);
}


void Change_Element(NODE_PTR Element_Ptr, Scalar value)
{       
   /* change the value in the node */

    Element_Ptr->element = value;
}
                                        

#if 0
void Print_Node(NODE_PTR Prt_Node)
{
    if (Prt_Node == NULL)
    {
            printf("NULL\n");
            return;
    }
    printf("Node element:%d\tcolumn:%d\n",Prt_Node->element,
            Prt_Node->column);
}
#endif


void Insert_Node(MAT_PTR Sparse_Matrix, int matrow, NODE_PTR New_Node, NODE_PTR Prev_Ptr)
{
    NODE_PTR temp_node;

    /* if it is to be the first element of the row */

    if (Prev_Ptr == NULL)
    {
        New_Node->Next_Node = Sparse_Matrix[matrow];
        Sparse_Matrix[matrow] = New_Node;
    }
  
    /* otherwise it has a predessor and possibly a successor*/

    else
    {
        temp_node = Prev_Ptr->Next_Node;
        Prev_Ptr->Next_Node = New_Node;
        New_Node->Next_Node = temp_node;
    }
};


void Delete_Node(MAT_PTR Sparse_Matrix, int RowId, NODE_PTR Prev_Ptr)
{
    NODE_PTR Bad_Node;
   
    /* It is the first element node in the row */     

    if (Prev_Ptr == NULL)
    {
        Bad_Node=(NODE_PTR) &(*Sparse_Matrix[RowId]);

        /* it is not the only one in the row */

        if (Bad_Node->Next_Node != NULL)
        {
            Sparse_Matrix[RowId]=Bad_Node->Next_Node;
        }

        /* it is the only one in the row */

        else
        {
            Sparse_Matrix[RowId]=NULL;
        }
    }
   
    /* There are other nodes before and after this one */

    else
    {
        Bad_Node = Prev_Ptr->Next_Node;
        Prev_Ptr->Next_Node=Bad_Node->Next_Node;
    }

    /* send the node back to the memory management system */

#pragma omp critical(record)
{
    PutRecord(Bad_Node);
}
};

bool Row_empty(MAT_PTR Sparse_Matrix, int row)
{
    return Sparse_Matrix[row] == NULL;
}

/**************************************************************************/
/* Locate_Node() either returns NULL or the node pointer to the node
   before the one we want. If it returns NULL then we know that 
   Either:
      1. Row empty or
      2. we are before the one we want and we are first or
      
   If it is non NULL then 
   Either:
      1. We are before the one we want and there is another node after us
      2. We are before the column we want and there are no nodes after us
*/
/**************************************************************************/

NODE_PTR Locate_Node(MAT_PTR Sparse_Matrix, int row, int col)
{
    NODE_PTR Prev_Ptr;
    NODE_PTR Curr_Ptr;

    Prev_Ptr = NULL;

    Curr_Ptr = Sparse_Matrix[row];

    /* We will either be at the end of the linked list or at
      the node just before the row,col that we want */

    while ((Curr_Ptr != NULL) && (col > Curr_Ptr->column))
    {
        Prev_Ptr=Curr_Ptr;
        Curr_Ptr=Curr_Ptr->Next_Node;
    }

    return(Prev_Ptr);
};

NODE_PTR Locate_Element(NODE_PTR Tmp_Ptr, int row, int col)
{
    NODE_PTR Prev_Ptr;
    NODE_PTR Curr_Ptr;

    Prev_Ptr = NULL;

    if (Tmp_Ptr==NULL)
    {
       Curr_Ptr=Matrix_Base_Ptr[row];
    }
    else
    {
       Curr_Ptr = Tmp_Ptr;
    }

    /* We will either be at the end of the linked list or at
      the node just before the row,col that we want */

    while ((Curr_Ptr != NULL) && (col > Curr_Ptr->column))
    {
        Prev_Ptr=Curr_Ptr;
        Curr_Ptr=Curr_Ptr->Next_Node;
    }

    return(Prev_Ptr);
};


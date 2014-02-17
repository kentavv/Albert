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

#include <list>
#include <vector>

using std::list;
using std::vector;

#include <stdio.h>
#include <stdlib.h>

#include <omp.h>

#include "SparseReduceMatrix.h"
#include "Build_defs.h"
#include "Scalar_arithmetic.h"

static void SparseMultRow(vector<list<Node> > &SM, int Row, Scalar Factor);
static void SparseAddRow(vector<list<Node> > &SM, Scalar Factor, int Row1, int Row2);
static void SparseKnockOut(vector<list<Node> > &SM, int row, int col, int nRows);
#if 0
static void Print_Matrix(MAT_PTR Sparse_Matrix, int r, int c);
static void Print_Rows(int Row1, int Row2, int nCols);
static void Print_SLList(Node *SLHead_Ptr);
static void Print_Node(NODE_PTR Prt_Node);
#endif


#include <time.h>
#include <sys/times.h>

int SparseReduceMatrix(vector<list<Node> > &SM, int nRows, int nCols, int *Rank)
{
    if(nRows == 0 || nCols == 0)
    {
        return(OK);
    }

{
  putchar('\n');
  int n = 0;
  int nz = 0;
  int nnz = 0;
  int nrz = 0;
  int nzc = 0;
  for(int i=0; i<nRows; i++) {
    if(SM[i].empty()) nrz++; 
    n += SM[i].size();
    list<Node>::const_iterator ii;
//  printf("%d %d\n", i, SM[i].size());
    for(ii=SM[i].begin(); ii!=SM[i].end(); ii++) {
      if(i == 2636) {
        printf("<%d %d>", ii->column, ii->element);
      }
      if(ii->element == 0) nz++;
      if(ii->element != 0) nnz++;
      if(ii->column == 0) nzc++;
    }
  }
  printf("A n:%d nz:%d nrz:%d nnz:%d nzc:%d nRows:%d nCols:%d\n", n, nz, nrz, nnz, nzc, nRows, nCols);
}
    /* Search for the rightmost nonzero element */
    /* Dependent on the current stairrow */

    int nextstairrow = 0;
    for (int i=0;i<nCols;i++)
    {
//struct tms aa, bb, cc;
//times(&aa);
        int j;
        for (j=nextstairrow; j < nRows; j++)
        {
            if(Get_Matrix_Element(SM, j,i) != S_zero())
            {
                break;
            }
        }
//times(&bb);
        /* When found interchange and then try to knockout any nonzero
           elements in the same column */

        if (j < nRows)
        {
           SM[nextstairrow].swap(SM[j]);
           SparseKnockOut(SM, nextstairrow, i, nRows);
           nextstairrow++;
        }
//times(&cc);
//long uaa=aa.tms_utime;
//long ubb=bb.tms_utime;
//long ucc=cc.tms_utime;
//long saa=aa.tms_stime;
//long sbb=bb.tms_stime;
//long scc=cc.tms_stime;
//printf("%ld %ld %ld  %ld %ld %ld\t\t", uaa, ubb, ucc, ubb-uaa, ucc-uaa, ucc-ubb);
//printf("%ld %ld %ld  %ld %ld %ld\n", saa, sbb, scc, sbb-saa, scc-saa, scc-sbb);
    }
    *Rank=nextstairrow;

{
  int n = 0;
  int nz = 0;
  int nnz = 0;
  int nrz = 0;
  int nzc = 0;
  for(int i=0; i<nRows; i++) {
    if(SM[i].empty()) nrz++;
    n += SM[i].size();
    list<Node>::const_iterator ii;
    for(ii=SM[i].begin(); ii!=SM[i].end(); ii++) {
      if(i == 2636) {
        printf("<%d %d>", ii->column, ii->element);
      }
      if(ii->element == 0) nz++;
      if(ii->element != 0) nnz++;
      if(ii->column == 0) nzc++;
    }
  }
  printf("B n:%d nz:%d nrz:%d nnz:%d nzc:%d nRows:%d nCols:%d\n", n, nz, nrz, nnz, nzc, nRows, nCols);
}

    return(OK);
}


void SparseMultRow(vector<list<Node> > &SM, int Row, Scalar Factor)
{
   /* Step thru row ... multiplying each element by the factor */
   for(list<Node>::iterator ii = SM[Row].begin(); ii != SM[Row].end(); ii++) {
      ii->element = S_mul(ii->element, Factor);
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
void SparseAddRow(vector<list<Node> > &SM, Scalar Factor, int Row1, int Row2)
{
  /* check for zero factor */

  if (Factor == S_zero()) {
    return;
  }

  /* get the beginning of the two rows to work with */

  const list<Node> &r1 = SM[Row1];
  list<Node> &r2 = SM[Row2];

  list<Node>::const_iterator r1i = r1.begin();
  list<Node>::iterator r2i = r2.begin();

  for(; r1i != r1.end() && r2i != r2.end();) {
    if(r1i->column == r2i->column) {
      Scalar x = S_add(r2i->element, S_mul(Factor, r1i->element));
      if(x == S_zero()) {
        r1i++;
        r2i = r2.erase(r2i);
      } else {
        r2i->element = x;
        r1i++;
        r2i++;
      }
    } else if(r1i->column < r2i->column) {
      Scalar x = S_mul(Factor, r1i->element);
      //if(x != S_zero()) {
        Node n = *r1i;
        n.element = x;
        r2i = r2.insert(r2i, n);
      //}
      r1i++;
    } else { //if(r1i->column > r2i->column) {
      r2i++;
    }
  }

  // append r2 with remaining r1 nodes
  for(; r1i != r1.end(); r1i++) {
    Scalar x = S_mul(Factor, r1i->element);
    //if(x != S_zero()) {
      Node n = *r1i;
      n.element = x;
      r2.push_back(n);
    //}
  }
}

void SparseKnockOut(vector<list<Node> > &SM, int row, int col, int nRows)
{
    Scalar x = Get_Matrix_Element(SM, row, col);
    if(x != S_one())
    {
    /* if the rightmost element in the current row is not one then multiply*/
        SparseMultRow(SM, row, S_inv(x));
    }

    /* try to knockout elements in column in the rows above */ 

#pragma omp parallel for schedule(dynamic, 10)
    for (int j=0; j < nRows; j++) {
      if(j != row) {
        SparseAddRow(SM, S_minus(Get_Matrix_Element(SM, j, col)), row, j);
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

Scalar Get_Matrix_Element(const vector<list<Node> > &SM, int i, int j)
{
    /* either return the element at location i,j or return a zero */
    for(list<Node>::const_iterator ii = SM[i].begin(); ii != SM[i].end() && ii->column <= j; ii++) {
      if(ii->column == j) return ii->element;
    }
    return S_zero();
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

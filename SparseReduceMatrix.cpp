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
#include <algorithm>

using std::list;
using std::vector;
using std::lower_bound;
//using std::random_shuffle;

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <omp.h>

#include "SparseReduceMatrix.h"
#include "Build_defs.h"
#include "Scalar_arithmetic.h"

static void SparseMultRow(SparseMatrix &SM, int Row, Scalar Factor);
static void SparseAddRow(SparseMatrix &SM, Scalar Factor, int Row1, int Row2);
static void SparseKnockOut(SparseMatrix &SM, int row, int col);
#if 0
static void Print_Matrix(MAT_PTR Sparse_Matrix, int r, int c);
static void Print_Rows(int Row1, int Row2, int nCols);
static void Print_SLList(Node *SLHead_Ptr);
static void Print_Node(NODE_PTR Prt_Node);
#endif

struct stats {
  //size_t n_zero_elements;
  size_t n_elements;
  size_t capacity;
  size_t n_zero_rows;
  size_t n_rows;
  size_t n_cols;
  int last_nextstairrow;
  int prev_col;
  int cur_col;
  time_t first_update;
  time_t prev_update;
  time_t cur_update;

  stats() : // n_zero_elements(0),
    n_elements(0),
    capacity(0),
    n_zero_rows(0),
    n_rows(0),
    n_cols(0),
    last_nextstairrow(0),
    prev_col(0),
    cur_col(0),
    first_update(0),
    prev_update(0),
    cur_update(0) {}

  void clear() {
    //n_zero_elements = 0;
    n_elements = 0;
    capacity = 0;
    n_zero_rows = 0;
    n_rows = 0;
    n_cols = 0;
    last_nextstairrow = 0;
    // These are not reset between updates because they are used
    // to calculate rates.
    //prev_col = 0;
    //cur_col = 0;
    //first_update = 0;
    //prev_update = 0;
    //cur_update = 0;
  }
 
  static void tp(float t) {
    if(t > 3600) {
      printf("%.02fh", t / 3600.);
    } else if(t > 60) {
      printf("%.02fm", t / 60.);
    } else {
      printf("%.02fs", t);
    }
  }
 
  void print() const {
    printf("\r\t\tne:%lu (%.1fMB)", n_elements, n_elements * sizeof(Node) / 1024. / 1024.);
#if 0
    if(n_zero_elements > 0) {
      printf(" ze:%lu", n_zero_elements);
    }
#endif
    if(n_elements != capacity) {
      printf(" ce:%lu", capacity);
    }
    printf("  zr:%lu  lr:%d/%lu  lc:%d/%lu",
           n_zero_rows,
           last_nextstairrow, n_rows,
           cur_col, n_cols);
    {
      time_t dt = cur_update - first_update;
      if(dt > 0) {
        printf("  tt:"); tp(dt);
      }
    }
    if(cur_col > 100) {
      int dt = cur_update - prev_update;
      if(dt != 0) {
        float cps = (cur_col - prev_col + 1) / float(dt);
        printf("  cps:%.02f", cps);

        float eta = (n_cols - cur_col) / cps;
        if(eta > 1) {
          printf("  etr:"); tp(eta);
        }
      }
    }
    printf("                    ");
    fflush(NULL);
  }

  void update(const SparseMatrix &SM, int nextstairrow_, int last_col_, int nCols_, int timeout=-1, bool do_print=false) {
    time_t t = time(NULL);
    if(timeout != -1 && cur_update != 0 && t - cur_update < timeout) {
      return;
    }

    clear();
    if(first_update == 0) {
      first_update = t;
    }
    prev_update = cur_update;
    cur_update = t;

    n_rows = SM.size();
    n_cols = nCols_;
    last_nextstairrow = nextstairrow_;
    prev_col = cur_col;
    cur_col = last_col_;

    for(int ii=0; ii<(int)SM.size(); ii++) {
      capacity += SM[ii].capacity();
      n_elements += SM[ii].size();

      if(SM[ii].size() == 0) {
        n_zero_rows++;
      }

#if 0
      // There should be no zero elements
      for(int jj=0; jj<(int)SM[ii].size(); jj++) {
        if(SM[ii][jj].getElement() == S_zero()) {
          n_zero_elements++;
        }
      }
#endif
    }

    if(do_print) {
      print();
    }
  }
};


int SparseReduceMatrix(SparseMatrix &SM, int nCols, int *Rank)
{
    if(SM.empty() || nCols == 0)
    {
        return(OK);
    }

//random_shuffle(SM.begin(), SM.end());

    //printf("s:%d c:%d ", (int)SM.size(), (int)SM.capacity());
    /* Search for the rightmost nonzero element */
    /* Dependent on the current stairrow */

    stats s1;
    s1.update(SM, 0, 0, nCols, -1, true);

    int nextstairrow = 0;
    for (int i=0;i<nCols;i++)
    {
        int j;
        for (j=nextstairrow; j < (int)SM.size(); j++)
        {
            if(Get_Matrix_Element(SM, j,i) != S_zero())
            {
                break;
            }
        }
        /* When found interchange and then try to knockout any nonzero
           elements in the same column */

        if (j < (int)SM.size())
        {
           SM[nextstairrow].swap(SM[j]);
           SparseKnockOut(SM, nextstairrow, i);
           nextstairrow++;
        }

        s1.update(SM, nextstairrow, i, nCols, 60, true);
    }
    *Rank=nextstairrow;
    s1.update(SM, nextstairrow, nCols, nCols, -1, true);

    putchar('\n');

    return(OK);
}


void SparseMultRow(SparseMatrix &SM, int Row, Scalar Factor)
{
   /* Step thru row ... multiplying each element by the factor */
   for(SparseRow::iterator ii = SM[Row].begin(); ii != SM[Row].end(); ii++) {
      ii->setElement(S_mul(ii->getElement(), Factor));
   }
}

template< typename T, class Allocator >
void shrink_capacity(std::vector<T,Allocator>& v)
{
   std::vector<T,Allocator>(v.begin(),v.end()).swap(v);
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
void SparseAddRow(SparseMatrix &SM, Scalar Factor, int Row1, int Row2)
{
  /* check for zero factor */

  if (Factor == S_zero()) {
    return;
  }

  /* get the beginning of the two rows to work with */

  const SparseRow &r1 = SM[Row1];
  SparseRow &r2 = SM[Row2];

  SparseRow tmp;
  tmp.reserve(r1.size() + r2.size());

  SparseRow::const_iterator r1i = r1.begin();
  SparseRow::const_iterator r2i = r2.begin();

  for(; r1i != r1.end() && r2i != r2.end();) {
    if(r1i->getColumn() == r2i->getColumn()) {
      Scalar x = S_add(r2i->getElement(), S_mul(Factor, r1i->getElement()));
      if(x != S_zero()) {
        Node n = *r1i;
        n.setElement(x);
        tmp.push_back(n);
      }
      r1i++;
      r2i++;
    } else if(r1i->getColumn() < r2i->getColumn()) {
      Scalar x = S_mul(Factor, r1i->getElement());
      //if(x != S_zero()) {
        Node n = *r1i;
        n.setElement(x);
        tmp.push_back(n);
      //}
      r1i++;
    } else { //if(r1i->column > r2i->column) {
      tmp.push_back(*r2i);
      r2i++;
    }
  }

  // append r2 with remaining r1 nodes
  for(; r1i != r1.end(); r1i++) {
    Scalar x = S_mul(Factor, r1i->getElement());
    //if(x != S_zero()) {
      Node n = *r1i;
      n.setElement(x);
      tmp.push_back(n);
    //}
  }

  // append r2 with remaining r1 nodes
  for(; r2i != r2.end(); r2i++) {
    tmp.push_back(*r2i);
  }
  SparseRow(tmp.begin(), tmp.end()).swap(r2); // shrink capacity while assigning
  //r2 = SparseRow(tmp.begin(), tmp.end());
  //shrink_capacity(tmp);
  //printf("<%d %d %d %d>", (int)r2.size(), (int)r2.capacity(),  (int)tmp.size(), (int)tmp.capacity());
  //r2 = tmp; 
  //r2.swap(tmp); 
}

void SparseKnockOut(SparseMatrix &SM, int row, int col)
{
    Scalar x = Get_Matrix_Element(SM, row, col);
    if(x != S_one())
    {
    /* if the rightmost element in the current row is not one then multiply*/
        SparseMultRow(SM, row, S_inv(x));
    }

    /* try to knockout elements in column in the rows above */ 

#pragma omp parallel for schedule(dynamic, 10)
    for (int j=0; j < (int)SM.size(); j++) {
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

#if 0
static bool cmp_column(const Node &n1, const Node &n2) { return n1.column < n2.column; }

#if 0
struct A {
  bool operator()(const Node &n1, const Node &n2) const { return n1.column < n2.column; }
};
#endif

Scalar Get_Matrix_Element(const SparseMatrix &SM, int i, int j)
{
  Node n;
  n.column = j;

  SparseRow::const_iterator ii = lower_bound(SM[i].begin(), SM[i].end(), n, cmp_column);
  //SparseRow::const_iterator ii = lower_bound(SM[i].begin(), SM[i].end(), n, A());
  //SparseRow::const_iterator ii = lower_bound(SM[i].begin(), SM[i].end(), n);
  if(ii != SM[i].end() && ii->column == j) {
    return ii->element;
  }

  return S_zero();
}
#else
Scalar Get_Matrix_Element(const SparseMatrix &SM, int i, int j)
{
    /* either return the element at location i,j or return a zero */
    for(SparseRow::const_iterator ii = SM[i].begin(); ii != SM[i].end() && ii->getColumn() <= j; ii++) {
      if(ii->getColumn() == j) return ii->getElement();
    }
    return S_zero();
}
#endif

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

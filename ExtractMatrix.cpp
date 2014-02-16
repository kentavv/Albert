/*******************************************************************/
/***  FILE :        ExtractMatrix.c                              ***/
/***  AUTHOR:       David P Jacobs                               ***/
/***  PROGRAMMER:   Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990.                                    ***/
/***  MODIFIED    : David Lee (8/92)                             ***/
/***                   - Added code to extract  information      ***/
/***                     from a sparse matrix.                   ***/
/***                9/93 - Trent Whiteley                        ***/
/***                       switched variables of type Term_list  ***/
/***                       to type Term *                        ***/
/***  PUBLIC ROUTINES:                                           ***/
/***                     SparseExtractMatrix()                   ***/
/***  PRIVATE ROUTINES:                                          ***/
/***                     SparseFillDependent()                   ***/
/***                     SparseProcessDependentBasis()           ***/
/***  MODULE DESCRIPTION:                                        ***/
/*******************************************************************/

#include <vector>

#include <stdio.h>
#include <stdlib.h>

using namespace std;

#include "ExtractMatrix.h"
#include "Basis_table.h"
#include "Build_defs.h"
#include "CreateMatrix.h"
#include "Memory_routines.h"
#include "Mult_table.h"
#include "Scalar_arithmetic.h"
#include "Sparse_structs.h"
#include "Sparse_defs.h"
#include "SparseReduceMatrix.h"
#include "Type_table.h"

static void SparseFillDependent(vector<int> &Dependent);
static void FillDependent(vector<int> &Dependent);
#if 0
static void PrintDependent(void);
#endif
static void ProcessIndependentBasis(const vector<int> &Dependent, vector<Basis> &BasisNames);
static void ProcessDependentBasis(const vector<int> &Dependent, vector<Basis> &BasisNames);
static void SparseProcessDependentBasis(vector<Basis> &BasisNames);
static void ProcessOtherIndependentBasis(int J);

static Type Cur_type;
static Type T1;
static Type T2;
static int Cur_type_degree;
static int Cur_type_len;
static int Num_rows;
static int Num_cols;
static int MatrixRank;
static Matrix TheMatrix = NULL;
static MAT_PTR TheSparseMatrix = NULL;
static Unique_basis_pair_list ColtoBP;

int ExtractFromTheMatrix(Matrix Mptr, int Rows, int Cols, int Rank, Name N, Unique_basis_pair_list BPCptr)
{
    TheMatrix = Mptr; 
    Num_rows = Rows;
    Num_cols = Cols;
    MatrixRank = Rank;
    ColtoBP = BPCptr;

    Cur_type_len = GetTargetLen();
    Cur_type = (Type) Mymalloc(Cur_type_len * sizeof(Degree));
    assert_not_null(Cur_type);

    T1 = (Type) Mymalloc((Cur_type_len + 2) * sizeof(Degree)); /* +2 to avoid buffer overflow */
    assert_not_null(T1);

    NameToType(N,T1);
    NameToType(N,Cur_type);

    Cur_type_degree = GetDegree(Cur_type);
    T2 = (Type) Mymalloc(Cur_type_len * sizeof(Degree));
    assert_not_null(T2);

    if (Num_cols > 0 ) {
        vector<int> Dependent(Num_cols, 0);
        vector<Basis> BasisNames(Num_cols, 0);

        FillDependent(Dependent);
        ProcessIndependentBasis(Dependent, BasisNames);
        ProcessDependentBasis(Dependent, BasisNames);
    }

    ProcessOtherIndependentBasis(0);

    free(Cur_type);
    free(T1);
    free(T2);

    return(OK);
}


/* Added (8/92) by DCL. This is virtually identical to ExtractFromMatrix()
   except for the calls to SparseFillDependent() and SparseProcessDependent-
   Basis(). Operation are performed on a locally visible pointer to the 
   matrix and then that pointer is copied to the one passed in upon exit */
   
int SparseExtractFromMatrix(MAT_PTR SMptr, int Rows, int Cols, int Rank, Name N, Unique_basis_pair_list BPCptr)
{
    TheSparseMatrix = SMptr; 
    Num_rows = Rows;
    Num_cols = Cols;
    MatrixRank = Rank;
    ColtoBP = BPCptr;

    Cur_type_len = GetTargetLen();
    Cur_type = (Type) Mymalloc(Cur_type_len * sizeof(Degree));
    assert_not_null(Cur_type);

    T1 = (Type) Mymalloc((Cur_type_len + 2) * sizeof(Degree)); /* +2 to avoid buffer overflow */
    assert_not_null(T1);

    NameToType(N,T1);
    NameToType(N,Cur_type);

    Cur_type_degree = GetDegree(Cur_type);
    T2 = (Type) Mymalloc(Cur_type_len * sizeof(Degree));
    assert_not_null(T2);

    if (Num_cols > 0 ) {
        vector<int> Dependent(Num_cols, 0);
        vector<Basis> BasisNames(Num_cols, 0);

        SparseFillDependent(Dependent);
        ProcessIndependentBasis(Dependent, BasisNames);
        SparseProcessDependentBasis(BasisNames);
    }

    ProcessOtherIndependentBasis(0);

    free(Cur_type);
    free(T1);
    free(T2);

    return(OK);
}


void SparseFillDependent(vector<int> &Dependent)
{
    if ((Num_rows == 0) || (Num_cols == 0))
        return;

         /* This routine is much simpler than its sister FillDependent()
            since the first node in the linked list of each row will contain
            the first nonzero element */

             /* place ones in the correct columns of the Dependent structure */
    for(int i=0; i < MatrixRank; i++)
	 {

             Dependent[TheSparseMatrix[i]->column] = S_one();
	 } 

}

void FillDependent(vector<int> &Dependent)
{
    if ((Num_rows == 0) || (Num_cols == 0))
        return;

    for (int i=0;i<MatrixRank;i++) {
        for (int j=0;j<Num_cols;j++) {
            if (TheMatrix[i*Num_cols + j] > 0) {
               Dependent[j] = 1;
               break;
            }
        }
    }
}


#if 0
void PrintDependent(void)
{
    int j;

    if (Num_cols == 0)
        return;

    printf("The Dependent Array is : \n");
    for (j=0;j<Num_cols;j++)
        printf("%d",Dependent[j]);
    printf("\n");
}
#endif


void ProcessIndependentBasis(const vector<int> &Dependent, vector<Basis> &BasisNames)
{
    if (Num_cols == 0)
        return;

    vector<pair<Basis, Scalar> > tl(1);

    for (int j=0;j<Num_cols;j++) {
        if (!Dependent[j]) {
            Basis b1 = ColtoBP[j].left_basis;
            Basis b2 = ColtoBP[j].right_basis;
            Basis n = EnterBasis(b1,b2,TypeToName(Cur_type));
            tl[0] = make_pair(n, 1);
            EnterProduct(b1, b2, tl);
            BasisNames[j] = n;
        }
    }
}

             

void ProcessDependentBasis(const vector<int> &Dependent, vector<Basis> &BasisNames)
{
    if (Num_cols == 0)
        return;

    vector<pair<Basis, Scalar> > tl;

    for (int j=0;j<Num_cols;j++) {
        if (Dependent[j]) {
            int row;
            for (row=0;row<MatrixRank;row++)
                if (TheMatrix[row*Num_cols + j] == 1)
                    break;
            Basis b1 = ColtoBP[j].left_basis;
            Basis b2 = ColtoBP[j].right_basis;
tl.clear();
            for (int k=j+1;k<Num_cols;k++) {
                if (TheMatrix[row*Num_cols + k] != 0) {
tl.push_back(make_pair(BasisNames[k], S_minus(TheMatrix[row*Num_cols + k])));
                }
            }
            EnterProduct(b1, b2, tl);
        }
    }
}

/* Again this is virtually identical to the sister routine of 
   ProcessDependentBasis except we get the information from the 
   sparse matrix structure. Also we do not use the dependent structure
   since the first element is each linked list representing a row is
   a nonzero element*/

void SparseProcessDependentBasis(vector<Basis> &BasisNames)
{
    if (Num_cols == 0)
        return;

   int rowid=0;

    vector<pair<Basis, Scalar> > tl;

    while (rowid < MatrixRank)
    {
       NODE_PTR Tmp_Ptr = TheSparseMatrix[rowid];
       int j=Tmp_Ptr->column;
 tl.clear();
       NODE_PTR q=NULL;
       if (Tmp_Ptr->Next_Node != NULL)
       {
	    q=Tmp_Ptr->Next_Node;
       }
       while (q)
       {
           int k=q->column;
	   Scalar S_temp=Get_Matrix_Element(TheSparseMatrix,rowid,k);
           tl.push_back(make_pair(BasisNames[k], S_minus(S_temp)));
	   q = q->Next_Node;	
    	 }

       Basis b1 = ColtoBP[j].left_basis;
       Basis b2 = ColtoBP[j].right_basis;
       EnterProduct(b1, b2, tl);

	    rowid++;
    	 Tmp_Ptr = TheSparseMatrix[rowid];
    }
}

void ProcessOtherIndependentBasis(int J)
{
   int i,deg,save,/*num_bp,*/j;
   Basis m1,m2,n1,n2,n;
    vector<pair<Basis, Scalar> > tl(1);

    if (Cur_type_len == J) {
        deg = GetDegree(T1);
        if ((deg > 0) && (deg < Cur_type_degree)) {
            for (i=0;i<Cur_type_len;i++)
                T2[i] = Cur_type[i] - T1[i];
            m1 = BeginBasis(TypeToName(T1));
            m2 = EndBasis(TypeToName(T1));
            n1 = BeginBasis(TypeToName(T2));
            n2 = EndBasis(TypeToName(T2));
            if ((0 < m1) && (m1 <= m2) && (0 < n1) && (n1 <= n2)) {
                for (i=m1;i<=m2;i++) {
                    for (j=n1;j<=n2;j++) {
                        if (GetCol(i, j) == -1) {
                            n = EnterBasis(i,j,TypeToName(Cur_type));
  tl[0] = make_pair(n, 1);
                            EnterProduct(i, j, tl);
                        }
                    }
                }
            }
        }
    }
    else {
        for (i=0;i<=Cur_type[J];i++) {
            save = T1[i];
            T1[J] = i;
            ProcessOtherIndependentBasis(J+1);
            T1[i] = save;
        }
    }
} 

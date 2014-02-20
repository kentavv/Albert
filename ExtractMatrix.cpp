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

#include <list>
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
#include "SparseReduceMatrix.h"
#include "Type_table.h"

static void SparseFillDependent(const SparseMatrix &SM, vector<int> &Dependent);
#if 0
static void PrintDependent(void);
#endif
static void ProcessIndependentBasis(const vector<int> &Dependent, const vector<Unique_basis_pair> &ColtoBP, vector<Basis> &BasisNames);
static void SparseProcessDependentBasis(const SparseMatrix &SM, const vector<Unique_basis_pair> &ColtoBP, vector<Basis> &BasisNames);
static void ProcessOtherIndependentBasis(const vector<Unique_basis_pair> &ColtoBP, int J);

static Type Cur_type;
static Type T1;
static Type T2;
static int Cur_type_degree;
static int Cur_type_len;
static int Num_cols;
static int MatrixRank;

/* Added (8/92) by DCL. This is virtually identical to ExtractFromMatrix()
   except for the calls to SparseFillDependent() and SparseProcessDependent-
   Basis(). Operation are performed on a locally visible pointer to the 
   matrix and then that pointer is copied to the one passed in upon exit */
   
int SparseExtractFromMatrix(const SparseMatrix &SM, int Cols, int Rank, Name N, const vector<Unique_basis_pair> &ColtoBP)
{
    Num_cols = Cols;
    MatrixRank = Rank;

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

        SparseFillDependent(SM, Dependent);
        ProcessIndependentBasis(Dependent, ColtoBP, BasisNames);
        SparseProcessDependentBasis(SM, ColtoBP, BasisNames);
    }

    ProcessOtherIndependentBasis(ColtoBP, 0);

    free(Cur_type);
    free(T1);
    free(T2);

    return(OK);
}


void SparseFillDependent(const SparseMatrix &SM, vector<int> &Dependent)
{
    if (SM.empty() || (Num_cols == 0))
        return;

         /* This routine is much simpler than its sister FillDependent()
            since the first node in the linked list of each row will contain
            the first nonzero element */

             /* place ones in the correct columns of the Dependent structure */
    for(int i=0; i < MatrixRank; i++)
	 {

             Dependent[SM[i].begin()->column] = S_one();
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


void ProcessIndependentBasis(const vector<int> &Dependent, const vector<Unique_basis_pair> &ColtoBP, vector<Basis> &BasisNames)
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

             
/* Again this is virtually identical to the sister routine of 
   ProcessDependentBasis except we get the information from the 
   sparse matrix structure. Also we do not use the dependent structure
   since the first element is each linked list representing a row is
   a nonzero element*/

void SparseProcessDependentBasis(const SparseMatrix &SM, const vector<Unique_basis_pair> &ColtoBP, vector<Basis> &BasisNames)
{
    if (Num_cols == 0)
        return;

    vector<pair<Basis, Scalar> > tl;

    for(int rowId = 0; rowId < MatrixRank; rowId++) {
       const SparseRow &row = SM[rowId];

       int j=row.begin()->column;

       tl.clear();
       SparseRow::const_iterator ii = row.begin();
       for(ii++; ii!=row.end(); ii++) {
           int k = ii->column;
	   Scalar S_temp = Get_Matrix_Element(SM, rowId, k);
           tl.push_back(make_pair(BasisNames[k], S_minus(S_temp)));
       }

       Basis b1 = ColtoBP[j].left_basis;
       Basis b2 = ColtoBP[j].right_basis;
       EnterProduct(b1, b2, tl);
    }
}

void ProcessOtherIndependentBasis(const vector<Unique_basis_pair> &ColtoBP, int J)
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
                        if (GetCol(ColtoBP, i, j) == -1) {
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
            ProcessOtherIndependentBasis(ColtoBP, J+1);
            T1[i] = save;
        }
    }
} 

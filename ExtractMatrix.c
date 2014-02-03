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

#include <stdio.h>
#include <stdlib.h>

#include "ExtractMatrix.h"
#include "Build_defs.h"
#include "CreateMatrix.h"
#include "Memory_routines.h"
#include "Mult_table.h"
#include "Scalar_arithmetic.h"
#include "Sparse_structs.h"
#include "Sparse_defs.h"

static void SparseFillDependent(void);
static void FillDependent(void);
static int DestroyDependent(void);
static void PrintDependent(void);
static void ProcessIndependentBasis(void);
static void ProcessDependentBasis(void);
static void SparseProcessDependentBasis(void);
static void ProcessOtherIndependentBasis(int J);

static Type Cur_type;
static Type T1;
static Type T2;
static int Cur_type_degree;
static int Cur_type_len;
static int *Dependent;
static Basis *BasisNames;
static int Num_rows;
static int Num_cols;
static int MatrixRank;
static Matrix TheMatrix;
static MAT_PTR TheSparseMatrix;
static Unique_basis_pair_list ColtoBP;

int ExtractFromTheMatrix(Matrix Mptr, int Rows, int Cols, int Rank, Name N, Unique_basis_pair_list BPCptr)
{
    int len;
    int i;

    TheMatrix = Mptr; 
    Num_rows = Rows;
    Num_cols = Cols;
    MatrixRank = Rank;
    ColtoBP = BPCptr;

    Dependent = NULL;
    BasisNames = NULL;
 
    Cur_type_len = len = GetTargetLen();
    Cur_type = (Type) (Mymalloc(len*sizeof(Degree)));
    assert_not_null(Cur_type);
    T1 = (Type) (Mymalloc((len+1)*sizeof(Degree))); /* +1 to avoid buffer overflow */
    assert_not_null(T1);
    NameToType(N,T1);
    NameToType(N,Cur_type);
    Cur_type_degree = GetDegree(Cur_type);
    T2 = (Type) (Mymalloc(len*sizeof(Degree)));    
    assert_not_null(T2);
    if (Num_cols > 0 ) {
        Dependent = (int *) (Mymalloc(Num_cols*sizeof(int)));
        assert_not_null(Dependent);
        for (i=0;i<Num_cols;i++)
            Dependent[i] = 0;
        BasisNames = (Basis *) (Mymalloc(Num_cols*sizeof(Basis)));
        assert_not_null(BasisNames);
        for (i=0;i<Num_cols;i++)
            BasisNames[i] = 0;
        FillDependent();
        ProcessIndependentBasis();
        ProcessDependentBasis();
        DestroyDependent();
        free(BasisNames);
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
    int len;
    int i;

    TheSparseMatrix = SMptr; 
    Num_rows = Rows;
    Num_cols = Cols;
    MatrixRank = Rank;
    ColtoBP = BPCptr;

    Dependent = NULL;
    BasisNames = NULL;
 
    Cur_type_len = len = GetTargetLen();
    Cur_type = (Type) (Mymalloc(len*sizeof(Degree)));    
    assert_not_null(Cur_type);
    T1 = (Type) (Mymalloc((len+1)*sizeof(Degree))); /* +1 to avoid buffer overflow */
    assert_not_null(T1);
    NameToType(N,T1);
    NameToType(N,Cur_type);
    Cur_type_degree = GetDegree(Cur_type);
    T2 = (Type) (Mymalloc(len*sizeof(Degree)));    
    assert_not_null(T2);
    if (Num_cols > 0 ) {
        Dependent = (int *) (Mymalloc(Num_cols*sizeof(int)));
        assert_not_null(Dependent);
        for (i=0;i<Num_cols;i++)
            Dependent[i] = 0;
        BasisNames = (Basis *) (Mymalloc(Num_cols*sizeof(Basis)));
        assert_not_null(BasisNames);
        for (i=0;i<Num_cols;i++)
            BasisNames[i] = 0;
        SparseFillDependent();
        ProcessIndependentBasis();
        SparseProcessDependentBasis();
        DestroyDependent();
        free(BasisNames);
    }
    ProcessOtherIndependentBasis(0);
    free(Cur_type);
    free(T1);
    free(T2);
    return(OK);
}


void SparseFillDependent(void)
{
    int i=0;
	 NODE_PTR Tmp_Ptr;

    if ((Num_rows == 0) || (Num_cols == 0))
        return;

         /* This routine is much simpler than its sister FillDependent()
            since the first node in the linked list of each row will contain
            the first nonzero element */

	 while (i < MatrixRank)
	 {
             Tmp_Ptr = TheSparseMatrix[i];

             /* place ones in the correct columns of the Dependent structure */

             Dependent[Tmp_Ptr->column] = S_one();
             i++;
	 } 

}

void FillDependent(void)
{
    int i,j;

    if ((Num_rows == 0) || (Num_cols == 0))
        return;

    for (i=0;i<MatrixRank;i++) {
        for (j=0;j<Num_cols;j++) {
            if (TheMatrix[i*Num_cols + j] > 0) {
               Dependent[j] = 1;
               break;
            }
        }
    }
}


int DestroyDependent(void)
{
    assert_not_null(Dependent);
    free(Dependent);
    return(OK);
}


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

void ProcessIndependentBasis(void)
{
    int j;
    Basis n,b1,b2;
    Term *tl = Alloc_Terms_list();	/* TW 9/22/93 - Terms_list change */

    assert_not_null_nv(tl);		/* TW 9/22/93 - Terms_list change */
    
    if (Num_cols == 0)
        return;

    for (j=0;j<Num_cols;j++) {
        if (!Dependent[j]) {
            b1 = ColtoBP[j].left_basis;
            b2 = ColtoBP[j].right_basis;
            n = EnterBasis(b1,b2,TypeToName(Cur_type));
            tl[0].coef = 1;
            tl[0].word = n;
            tl[1].coef = tl[1].word = 0;
            EnterProduct(b1,b2,tl);
            BasisNames[j] = n;
        }
    }
    free(tl);				/* TW 9/27/93 - forgot to free it up */
}

             

void ProcessDependentBasis(void)
{
    int j,row,k;
    Basis n,b1,b2;
    int tl_index;
    Term *tl = Alloc_Terms_list();	/* TW 9/22/93 - Terms_list change */

    assert_not_null_nv(tl);		/* TW 9/22/93 - Terms_list change */

    if (Num_cols == 0)
        return;

    for (j=0;j<Num_cols;j++) {
        if (Dependent[j]) {
            for (row=0;row<MatrixRank;row++)
                if (TheMatrix[row*Num_cols + j] == 1)
                    break;
            b1 = ColtoBP[j].left_basis;
            b2 = ColtoBP[j].right_basis;
            tl_index = 0;
            for (k=j+1;k<Num_cols;k++) {
                if (TheMatrix[row*Num_cols + k] != 0) {
                    tl[tl_index].coef = S_minus(TheMatrix[row*Num_cols + k]);
                    tl[tl_index].word = BasisNames[k];
                    tl_index++;
                }
            }
            tl[tl_index].coef = tl[tl_index].word = 0;  
            EnterProduct(b1,b2,tl);
        }
    }
    free(tl);				/* TW 9/27/93 - forgot to free it up */
}

/* Again this is virtually identical to the sister routine of 
   ProcessDependentBasis except we get the information from the 
   sparse matrix structure. Also we do not use the dependent structure
   since the first element is each linked list representing a row is
   a nonzero element*/

void SparseProcessDependentBasis(void)
{
   Scalar S_temp;
   NODE_PTR Tmp_Ptr;
   NODE_PTR q;
   int rowid=0;

   short int j ,Row,k;
   Basis n,b1,b2;
   int tl_index;
   Term *tl = Alloc_Terms_list();	/* TW 9/22/93 - Terms_list change */

   assert_not_null_nv(tl);			/* TW 9/22/93 - Terms_list change */

    if (Num_cols == 0)
        return;

    while (rowid < MatrixRank)
    {
       Tmp_Ptr = TheSparseMatrix[rowid];
       j=Tmp_Ptr->column;
       b1 = ColtoBP[j].left_basis;
       b2 = ColtoBP[j].right_basis;
       tl_index = 0;
       q=NULL;
       if (Tmp_Ptr->Next_Node != NULL)
       {
	    q=Tmp_Ptr->Next_Node;
       }
       while (q)
       {
           k=q->column;
	   S_temp=Get_Matrix_Element(TheSparseMatrix,rowid,k);
           tl[tl_index].coef = S_minus(S_temp);
           tl[tl_index].word = BasisNames[k];
           tl_index++;
	   q = q->Next_Node;	
    	 }
       tl[tl_index].coef = tl[tl_index].word = 0;  
       EnterProduct(b1,b2,tl);
	    rowid++;
    	 Tmp_Ptr = TheSparseMatrix[rowid];
    }
    free(tl);				/* TW 9/27/93 - forgot to free it up */
}

void ProcessOtherIndependentBasis(int J)
{
   int i,deg,save,num_bp,j;
   Basis m1,m2,n1,n2,n;
   Term *tl = Alloc_Terms_list();	/* TW 9/22/93 - Terms_list change */

   assert_not_null_nv(tl);			/* TW 9/22/93 - Terms_list change */

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
                        if (GetCol(i,j) == -1) {
                            n = EnterBasis(i,j,TypeToName(Cur_type));
                            tl[0].coef = 1;
                            tl[0].word = n;
                            tl[1].coef = tl[1].word = 0;
                            EnterProduct(i,j,tl);
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
    free(tl);				/* TW 9/27/93 - forgot to free it up */
} 

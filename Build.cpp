/******************************************************************/
/***  FILE :        Build.c                                     ***/
/***  AUTHOR:       David P. Jacobs                             ***/
/***  PROGRAMMER:   Sekhar Muddana                              ***/
/***  DATE WRITTEN: May 1990.                                   ***/
/***  MODIFIED:     Aug 1992. David Lee.                        ***/
/***                           Sparse Matrix Code Added.        ***/
/***                10/93 - Trent Whiteley                      ***/
/***                        changes made to implement interrupt ***/
/***                        handler                             ***/
/***  PUBLIC ROUTINES:                                          ***/
/***      int BuildDriver()                                     ***/ 
/***  PRIVATE ROUTINES:                                         ***/
/***      int InitializeStructures()                            ***/ 
/***      int DestroyStructures()                               ***/ 
/***      int PrintProgress()                                   ***/
/***      int ProcessDegree()                                   ***/
/***      int ProcessType()                                     ***/
/***      int SolveEquations()                                  ***/
/***  MODULE DESCRIPTION:                                       ***/
/***      Implement the Build Command.                          ***/
/***      Reads the sparse global variable to determine         ***/
/***      whether the traditional or sparse code should be used ***/
/***      in the SolveEquations routine.                        ***/
/******************************************************************/

#include <stdio.h>
#include <time.h>

#include "Build.h"
#include "Build_defs.h"
#include "Basis_table.h"
#include "ExtractMatrix.h"
#include "GenerateEquations.h"
#include "Mult_table.h"
#include "node_mgt.h"
#include "CreateMatrix.h"
#include "Po_parse_exptext.h"
#include "Id_routines.h"
#include "ReduceMatrix.h"
#include "Sparse_structs.h"
#include "Sparse_defs.h"
#include "SparseReduceMatrix.h"
#include "Debug.h"

static int InitializeStructures(void);
static void DestroyStructures(void);
static long ElapsedTime(void);
static void PrintProgress(int i, int n);
static int ProcessDegree(int i);
static void InstallDegree1(void);
static int ProcessType(Name n);
static int SolveEquations(Eqn_list_node *L /* Linked list of pair lists */, Name n);

/*****************************
 These variables are for the sparse implementation
******************************/
extern int sparse;
int gather_density_flag;
long num_elements;
long max_num_elements;
static long matrix_size;

extern int sigIntFlag;		/* TW 10/8/93 - flag for Ctrl-C */

static Type Target_type;
static struct id_queue_node *First_id_node;

static time_t Start_time; 
static Basis Current_dimension;

/*******************************************************************/
/* MODIFIES:                                                       */
/*     Mult_table -- is fully built.                               */
/*     Basis_table -- will be Complete.                            */ 
/* REQUIRES:                                                       */
/*     Target_Type -- Target to be reached.                        */
/*     Identities -- All the identities entered by User.           */
/* FUNCTION:                                                       */
/*     This is the highest level function in this part of the      */
/*     system. It gets called from the command interpretter when   */
/*     "build" is invoked.                                         */
/*     For each degree, New Basis are created and New Products are */
/*     entered.                                                    */ 
/*******************************************************************/
int Build(struct id_queue_node *Idq_node, Type Ttype)
{
    char *convtime;

    int Target_degree;

    int status = OK;
    int i;

    double density;

    Start_time = time(NULL);
    convtime = ctime(&Start_time);
    printf("\nBuild begun at %s\n",convtime);
    printf("Degree    Current Dimension   Elapsed Time(in seconds) \n");

    Target_type = Ttype;

    First_id_node = Idq_node;

    status = InitializeStructures();

    Target_degree = GetDegreeName(TypeToName(Target_type));

    gather_density_flag = FALSE;	
    if (status == OK) {
        for (i=1; i <= Target_degree; i++)  {
	if (i == Target_degree)
           gather_density_flag = TRUE;	
		
            status = ProcessDegree(i);
	    if(sigIntFlag == 1){
/*	      printf("Returning from Build().\n");*/
	      return(-1);
	    }
            if (status != OK) 
                break;
            PrintProgress(i, Target_degree);
        }
    }
#if PRINT_BASIS_TABLE
    PrintBasisTable();
#endif

/*#if PRINT_TYPE_TABLE
    PrintTypetable();
#endif

#if DEBUG_MT
    PrintMT();
#endif*/

#if PRINT_MULT_TABLE
    Print_MultTable();
#endif

    DestroyStructures();
    density= 100 * ((double)max_num_elements/(double)matrix_size);
    if (status == OK)
    {
        printf("Build completed. ");
        if (density > 0.00)
        {
            printf("Last Matrix %2.2f%% dense.\n",density);
        }
        else
        {
           if (matrix_size==0)
           {
              printf("No Matrix.\n");
           }
           else
           {
             printf("Last Matrix %f%%dense.\n",density);
           }
        }
    }
    else
        printf("Build incomplete\n");
    return (status);
}


/*******************************************************************/
/* GLOBALS INITIALIZED:                                            */
/*      Scalar Inverse_table -- For Scalar Arithemetic.            */
/*      Mult_table -- Multiplication Table.                        */
/*      Type_table -- Type Table for the Given Target Type.        */
/*      Basis_table -- to 0's.                                     */
/*******************************************************************/
int InitializeStructures(void)
{
    int status = OK; 

    int Target_degree;

    if (status == OK) 
        status = CreateTypeTable(Target_type);
    if (status == OK) { 
        Target_degree = GetDegreeName(TypeToName(Target_type));
        status = CreateBasisTable(Target_degree);
    }
    return (status);
}


void DestroyStructures(void)
{
     /*DestroyTypeTable();*/ /* if we delete this we will not be able to use the 'v b' command */
}


long ElapsedTime(void) {
    return time(NULL) - Start_time;
}
  
void PrintProgress(int i, int n)
{
    printf("  %2d/%2d           %4d            %5ld\n",
           i, n, Current_dimension, ElapsedTime());
}


/*******************************************************************/
/* REQUIRES:                                                       */
/*     i -- to process degree i.                                   */ 
/* FUNCTION:                                                       */
/*     Process all Types of degree i.                              */
/*******************************************************************/
int ProcessDegree(int i)
{
   Name n;
   int status = OK;
   Basis begin_basis;
   Basis end_basis = 0;

   if (i == 1)
       InstallDegree1();
   else {
       int nn1 = 0;
       int nn2 = 0;
       {
         n = FirstTypeDegree(i);
         while ((status == OK) && (n != -1)) {
           n = NextTypeSameDegree(n);
           nn2++;  
         }
       }

       n = FirstTypeDegree(i);
       while ((status == OK) && (n != -1)) {
           begin_basis = GetNextBasisTobeFilled();
           printf("\tProcessing(%2d/%2d, begin_basis:%d)...", (nn1++)+1, nn2, begin_basis); fflush(NULL);
           status = ProcessType(n);
	   if(sigIntFlag == 1){	/* TW 10/5/93 - Ctrl-C check */
/*	     printf("Returning from ProcessDegree().\n");*/
	     return(-1);
	   }
           end_basis = GetNextBasisTobeFilled() - 1;
           if (end_basis < begin_basis)
               UpdateTypeTable(n,0,0);    /* No Basis table entries. */
           else
               UpdateTypeTable(n,begin_basis,end_basis);
           n = NextTypeSameDegree(n);
       }
       Current_dimension = end_basis;
   }
   return(status);
}


/*******************************************************************/
/* REQUIRES: None.                                                 */
/* FUNCTION:                                                       */
/*     All Degree 1 Basis i.e Generators are entered into Basis    */
/*     Table.                                                      */ 
/*******************************************************************/
void InstallDegree1(void)
{
    int i,j;
    int len;
    Name n;
    Basis begin_basis;
    Basis end_basis = 0;
    Type temp_type;

    temp_type = GetNewType();
    
    len = GetTargetLen();

    for (i=0;i<len;i++) {
        for (j=0;j<len;j++)
            temp_type[j] = 0;
        temp_type[i] = 1;
        n = TypeToName(temp_type);
        begin_basis = GetNextBasisTobeFilled();
        EnterBasis(0,0,n);
        end_basis = GetNextBasisTobeFilled() - 1;
        UpdateTypeTable(n,begin_basis,end_basis);
    }
    Current_dimension = end_basis;
}


/*******************************************************************/
/* REQUIRES:                                                       */
/*     t -- to process Type t.                                     */ 
/* FUNCTION:                                                       */
/*     For each identity f, whose degree is less than the degree   */
/*     of type t, generate equations corresponding to f.           */
/*     Then solve those equations, to get New Baisis and write     */
/*     other basis pairs in terms of existing basis.               */ 
/*******************************************************************/
/* Process type t for degree i */
int ProcessType(Name n)
{
    int status = OK;
    Eqn_list_node *L;               /* Header record of linked list */
    struct polynomial *f;
    struct id_queue_node *temp_id_node;

    L = GetNewEqnListNode();
    assert_not_null(L);

    temp_id_node = First_id_node;

    printf("Generating..."); fflush(NULL);
    while (temp_id_node && status == OK) {
        f = temp_id_node->identity;
        if ((status == OK) && (f->degree <= GetDegreeName(n)))
            status = GenerateEquations(f,n,L);
	if(sigIntFlag == 1){		/* TW 10/5/93 - Ctrl-C check */
	  if(L != NULL){
	    FreeEqns(L);
	  }
/*	  printf("Returning from ProcessType().\n");*/
	  return(-1);
	}
        temp_id_node = temp_id_node->next;
    }

#if DEBUG_EQNS
    PrintEqns(L);
#endif

    printf("(%lds)...Solving...", ElapsedTime()); fflush(NULL);
    if (status == OK) 
        status = SolveEquations(L,n);			
    if (L != NULL)
        FreeEqns(L);

    printf("(%lds)\n", ElapsedTime());

    return(status);
}

/*******************************************************************/
/* REQUIRES:                                                       */
/*     L -- Head to List of Equations.                             */ 
/*     t -- Current Type being processed.                          */
/* FUNCTION:                                                       */
/*     Convert the given list of equations into Matrix, i.e one    */
/*     row for each equation and one column for each unique basis  */
/*     pair present in all equations.                              */
/*     Then Reduce that Matrix into row canonical form.            */
/*     Then Extract from the Reduced Matrix i.e Find New Basis     */
/*     and enter them into Basis Table. Then write Dependent Basis */
/*     pairs into Basis by entering products into Mult_table.      */ 
/*******************************************************************/
int SolveEquations(Eqn_list_node *L /* Linked list of pair lists */, Name n)
{
    int rows = 0;              /* Size of matrix */
    int cols = 0;              /* Size of matrix */
    Matrix mptr = NULL;        /* Pointer to matrix */
    MAT_PTR Sparse_Matrix = NULL;
    Unique_basis_pair_list BPCptr = NULL; /* pointer to BPtoCol */
    int rank = 0;
    int status = OK;


   /* this flag is only set on the last degree of the generator */

   if (gather_density_flag)
   {
      num_elements=0;
      max_num_elements=0;
      matrix_size=0;
   }
/* CreateMatrix will initialize rows, cols, m, BP
   and fill in matrix and BPtoCol */

/* determine the matrix structure */

   if (!sparse)
   {
     status = CreateTheMatrix(L, &mptr, &rows, &cols, &BPCptr, n);
   }
   else
   {
     status = SparseCreateTheMatrix(L, &Sparse_Matrix, &rows, &cols, &BPCptr, n);
   }

   /* make sure that we get the correct information from creating the 
      matrix for our statistics */

   if (gather_density_flag)
   {
      max_num_elements=num_elements;
      matrix_size=rows*cols;
   }


#if DEBUG_MATRIX
   PrintColtoBP();
   PrintTheMatrix();
#endif


/* determine the matrix structure */

   if (status == OK)
   {
/* determine the matrix structure */
      if (!sparse)
      {
#if 0
{
  int i, j, n=0;
  for(i=0; i<rows; i++) {
    for(j=0; j<cols; j++) {
      if(mptr[i * cols + j] != 0) {
        n++;
      }
    }
  }
printf("Matrix:(%4d X %4d, %d %d %f%%)", rows, cols, n, rows*cols, n/(double)(rows*cols)*100); fflush(NULL);
}
#else
printf("Matrix:(%4d X %4d)", rows, cols); fflush(NULL);
#endif
         status = ReduceTheMatrix(mptr, rows, cols, &rank);
      }
      else
      {
printf("Matrix(%4d X %4d)...", rows, cols); fflush(NULL);
         status = SparseReduceMatrix(&Sparse_Matrix,rows,cols,&rank);
      }
   }

#if DEBUG_MATRIX
   PrintTheRMatrix();
#endif

/* ExtractMatrix will expand basis table & MultTable ! */
  if (status == OK) 
  {
/* determine the matrix structure */
     if (!sparse)
     {
        status = ExtractFromTheMatrix(mptr, rows, cols, rank, n,BPCptr);	
     }
     else
     {
	status =SparseExtractFromMatrix(Sparse_Matrix,rows,cols,rank,n,BPCptr);
     }
 }
#if DEBUG_MATRIX
   PrintDependent();
#endif

   DestroyBPtoCol();     /* Make sure not null before malloc */
   if (status == OK) 
   {
      if (!sparse)
      {
         DestroyTheMatrix();
      }
      else
      {
         DestroySparseMatrix(Sparse_Matrix);
      }
   }
/* 
   DestroyDependent(); 
     This array was already freed!  This resulted in
     an error in which the same memory was freed, allocatted, freed
     and allocated, thus begin allocated to two different functions.
     This was discovered in June, 1993 by Sekhar.
*/
   return(status);
}

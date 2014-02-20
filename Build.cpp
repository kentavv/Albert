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

#include <list>
#include <vector>

using std::list;
using std::vector;

#include <stdio.h>
#include <time.h>

#include "Build.h"
#include "Build_defs.h"
#include "Basis_table.h"
#include "ExtractMatrix.h"
#include "GenerateEquations.h"
#include "Mult_table.h"
#include "CreateMatrix.h"
#include "Po_parse_exptext.h"
#include "Id_routines.h"
#include "SparseReduceMatrix.h"
#include "Debug.h"

static int InitializeStructures(Type Target_type);
static long ElapsedTime(void);
static void PrintProgress(int i, int n);
static int ProcessDegree(int i, const list<id_queue_node> &First_id_node);
static void InstallDegree1(void);
static int ProcessType(Name n, const list<id_queue_node> &First_id_node);
static int SolveEquations(Equations &equations, Name n);

extern int sigIntFlag;		/* TW 10/8/93 - flag for Ctrl-C */

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
int Build(list<id_queue_node> &Idq_node, Type Target_type)
{
    int status = OK;

    Start_time = time(NULL);
    const char *convtime = ctime(&Start_time);
    printf("\nBuild begun at %s\n", convtime);
    printf("Degree    Current Dimension   Elapsed Time(in seconds) \n");

    status = InitializeStructures(Target_type);

    int Target_degree = GetDegreeName(TypeToName(Target_type));
    if (status == OK) {
        for (int i=1; i <= Target_degree; i++)  {
            status = ProcessDegree(i, Idq_node);
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

/*
#if PRINT_TYPE_TABLE
    PrintTypetable();
#endif

#if DEBUG_MT
    PrintMT();
#endif
*/

#if PRINT_MULT_TABLE
    Print_MultTable();
#endif

    if (status == OK) {
        printf("Build completed.\n");
    } else {
        printf("Build incomplete.\n");
    }

    return (status);
}


/*******************************************************************/
/* GLOBALS INITIALIZED:                                            */
/*      Scalar Inverse_table -- For Scalar Arithemetic.            */
/*      Mult_table -- Multiplication Table.                        */
/*      Type_table -- Type Table for the Given Target Type.        */
/*      Basis_table -- to 0's.                                     */
/*******************************************************************/
int InitializeStructures(Type Target_type)
{
    int status = OK; 

    status = CreateTypeTable(Target_type);

    if (status == OK) { 
        int Target_degree = GetDegreeName(TypeToName(Target_type));
        status = CreateBasisTable(Target_degree);
    }

    return (status);
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
int ProcessDegree(int i, const list<id_queue_node> &First_id_node)
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
           printf("\tProcessing(%2d/%2d, begin_basis:%d)...", ++nn1, nn2, begin_basis); fflush(NULL);
           status = ProcessType(n, First_id_node);
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
int ProcessType(Name n, const list<id_queue_node> &First_id_node)
{
    int status = OK;
    Equations equations;

    printf("Generating..."); fflush(NULL);

    list<id_queue_node>::const_iterator ii = First_id_node.begin();
    for(; ii != First_id_node.end() && status == OK; ii++) {
        const polynomial *f = ii->identity;

        if(status == OK && f->degree <= GetDegreeName(n))
            status = GenerateEquations(f,n, equations);

	if(sigIntFlag == 1){		/* TW 10/5/93 - Ctrl-C check */
	  return(-1);
	}
    }

#if DEBUG_EQNS
    PrintEqns(equations);
#endif

    printf("(%lds)...Solving...", ElapsedTime()); fflush(NULL);
    if (status == OK) 
        status = SolveEquations(equations, n);			

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
int SolveEquations(Equations &equations, Name n)
{
    int cols = 0;              /* Size of matrix */
    vector<Unique_basis_pair> BPCptr; /* pointer to BPtoCol */
    int rank = 0;

/* CreateMatrix will initialize rows, cols, m, BP and fill in matrix and BPtoCol */

   SparseMatrix SM;
   int status = SparseCreateTheMatrix(equations, SM, &cols, BPCptr, n);

   equations.clear();

#if DEBUG_MATRIX
   PrintColtoBP();
   PrintTheMatrix();
#endif

   if (status == OK) {
     printf("Matrix:(%4d X %4d)", (int)SM.size(), cols); fflush(NULL);
     status = SparseReduceMatrix(SM,cols,&rank);
   }

#if DEBUG_MATRIX
   PrintTheRMatrix();
#endif

/* ExtractMatrix will expand basis table & MultTable ! */
  if (status == OK) {
	status = SparseExtractFromMatrix(SM,cols,rank,n, BPCptr);
 }
#if DEBUG_MATRIX
   PrintDependent();
#endif

   return(status);
}

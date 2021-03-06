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
using std::pair;

#include <stdio.h>
#include <string.h>
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
#include "SparseReduceMatrix2.h"
#include "SparseReduceMatrix3.h"
#include "SparseReduceMatrix4.h"
#include "SparseReduceMatrix7.h"
#include "matrix_reduce.h"
#include "matrix_reduce_avx.h"
#include "matrix_reduce_float.h"
#include "Debug.h"
#include "memory_usage.h"

#define TEST_SOLVERS 1

static int InitializeStructures(Type Target_type);

static long ElapsedTime();

static void PrintProgress(int i, int n);

static int ProcessDegree(int i, const list<id_queue_node> &First_id_node);

static void InstallDegree1();

static int ProcessType(Name n, const list<id_queue_node> &First_id_node, SparseMatrix &SM);

static int SolveEquations(SparseMatrix &SM, int cols, vector<Unique_basis_pair> &BPtoCol, Name n);

extern int sigIntFlag;        /* TW 10/8/93 - flag for Ctrl-C */

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

// Variables controlling saving images of the matrix as it's reduced.
bool __trigger = false;
bool __record = false;
int __deg = 0;
int __nn1 = 0;
int __nn2 = 0;

int Build(list<id_queue_node> &Idq_node, Type Target_type) {
    int status = OK;

    Start_time = time(nullptr);
    const char *convtime = ctime(&Start_time);
    printf("\nBuild begun at %s\n", convtime);
    printf("Degree    Current Dimension   Elapsed Time(in seconds) \n");

    status = InitializeStructures(Target_type);

    int Target_degree = GetDegreeName(TypeToName(Target_type));
    if (status == OK) {
        for (int i = 1; i <= Target_degree; i++) {
            // __record = trigger_ && i == Target_degree;
            __record = __trigger && i == 5;

            status = ProcessDegree(i, Idq_node);
            if (sigIntFlag == 1) {
/*	      printf("Returning from Build().\n");*/
                return -1;
            }
            if (status != OK) {
                break;
            }
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

    return status;
}


/*******************************************************************/
/* GLOBALS INITIALIZED:                                            */
/*      Scalar Inverse_table -- For Scalar Arithemetic.            */
/*      Mult_table -- Multiplication Table.                        */
/*      Type_table -- Type Table for the Given Target Type.        */
/*      Basis_table -- to 0's.                                     */
/*******************************************************************/
int InitializeStructures(Type Target_type) {
    int status = OK;

    status = CreateTypeTable(Target_type);

    if (status == OK) {
        status = CreateBasisTable();
    }

    return status;
}


long ElapsedTime() {
    return time(nullptr) - Start_time;
}

void PrintProgress(int i, int n) {
    printf("  %2d/%2d           %4d            %5ld\n",
           i, n,
           Current_dimension,
           ElapsedTime());
}


/*******************************************************************/
/* REQUIRES:                                                       */
/*     i -- to process degree i.                                   */
/* FUNCTION:                                                       */
/*     Process all Types of degree i.                              */
/*******************************************************************/
int ProcessDegree(int i, const list<id_queue_node> &First_id_node) {
    Name n;
    int status = OK;
    Basis begin_basis;
    Basis end_basis = 0;

    SparseMatrix SM;

    if (i == 1) {
        InstallDegree1();
    } else {
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
            printf("\tProcessing(%2d/%2d, begin_basis:%d)...", ++nn1, nn2, begin_basis);
            fflush(nullptr);

            __deg = i - 1;
            __nn1 = nn1;
            __nn2 = nn2;
            __record = __record && __nn1 == 1;

            status = ProcessType(n, First_id_node, SM);
            if (sigIntFlag == 1) {    /* TW 10/5/93 - Ctrl-C check */
/*	     printf("Returning from ProcessDegree().\n");*/
                return -1;
            }
            end_basis = GetNextBasisTobeFilled() - 1;
            if (end_basis < begin_basis) {
                UpdateTypeTable(n, 0, 0);    /* No Basis table entries. */
            } else {
                UpdateTypeTable(n, begin_basis, end_basis);
            }
            n = NextTypeSameDegree(n);
        }
        Current_dimension = end_basis;
    }
    return status;
}


/*******************************************************************/
/* REQUIRES: None.                                                 */
/* FUNCTION:                                                       */
/*     All Degree 1 Basis i.e Generators are entered into Basis    */
/*     Table.                                                      */
/*******************************************************************/
void InstallDegree1() {
    Basis end_basis = 0;

    Type temp_type = GetNewType();

    int len = GetTargetLen();

    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            temp_type[j] = 0;
        }
        temp_type[i] = 1;
        Name n = TypeToName(temp_type);
        Basis begin_basis = GetNextBasisTobeFilled();
        EnterBasis(0, 0, n);
        end_basis = GetNextBasisTobeFilled() - 1;
        UpdateTypeTable(n, begin_basis, end_basis);
    }
    Current_dimension = end_basis;

    free(temp_type);
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
int ProcessType(Name n, const list<id_queue_node> &First_id_node, SparseMatrix &SM) {
    int cols = 0;
    vector<Unique_basis_pair> BPtoCol;

    SM.clear();

    int status = OK;
    {
        Equations equations;

        printf("Generating...");
        fflush(nullptr);

        list<id_queue_node>::const_iterator ii = First_id_node.begin();
        for (; ii != First_id_node.end() && status == OK; ii++) {
            const polynomial *f = ii->identity;

            if (f->degree <= GetDegreeName(n)) {
                status = GenerateEquations(f, n, equations);
            }

            if (sigIntFlag == 1) {        /* TW 10/5/93 - Ctrl-C check */
                return -1;
            }
        }

        if (status == OK) {
#if 1
            {
                int tt = 0;
                for (int i = 0; i < (int) equations.size(); i++) {
                    tt += equations[i].size();
                }
                printf("neqn:%d (ne:%d MB:%.2f)...",
                       (int) equations.size(), tt,
                       tt * sizeof(Basis_pair) / 1024. / 1024.);
                fflush(nullptr);
            }
#endif

#if DEBUG_EQNS
            PrintEqns(equations);
#endif

            printf("(%lds)...Solving...", ElapsedTime());
            fflush(nullptr);
            status = SparseCreateTheMatrix(equations, SM, &cols, BPtoCol, n);

            //printf("BPtoCol:(%d MB:%.2f)...", (int)BPtoCol.size(), BPtoCol.size()*sizeof(Unique_basis_pair)/1024./1024.);
        }
    }

    if (status == OK) { /*SM.shrink_to_fit();*/
        status = SolveEquations(SM, cols, BPtoCol, n);
        printf("\t\tDone: %lds\n", ElapsedTime());
    }

    return status;
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
int SolveEquations(SparseMatrix &SM, int cols, vector<Unique_basis_pair> &BPtoCol, Name n) {
#if DEBUG_MATRIX
    PrintColtoBP();
    PrintTheMatrix();
#endif

    int tt = 0;
    for (int i = 0; i < (int) SM.size(); i++) {
        tt += SM[i].size();
    }
    int p_tt = tt;

    int rank = 0;
    int status = 0;
    printf("Matrix:(%d X %d)\n", (int) SM.size(), cols);

#if !TEST_SOLVERS
    status = SparseReduceMatrix(SM, cols, &rank);
#else
    SparseMatrix saved_SM = SM;

    if (cols == 12 || 1) {
        const int nfuncs = 8;
        int (*funcs[])(SparseMatrix &SM, int nCols, int *Rank) = {SparseReduceMatrix,
                                                                  SparseReduceMatrix2,
                                                                  SparseReduceMatrix3,
                                                                  SparseReduceMatrix4,
                                                                  SparseReduceMatrix5,
                                                                  SparseReduceMatrix6,
                                                                  SparseReduceMatrix7,
                                                                  SparseReduceMatrix8};
        const char *func_names[] = {"original",
                                    "column-major",
                                    "lazy-evaluation",
                                    "auto-sparse-dense",
                                    "truncated-dense",
                                    "truncated-dense-avx-float",
                                    "precompute-division-cache",
                                    "truncated-dense-avx"};
        bool include[] = {false,
                          false,
                          false,
                          false,
                          false,
                          false,
                          false,
                          true};

        if (getenv("ALBERT_METHODS")) {
            for (int i=0; i<sizeof(include); i++) {
                include[i] = false;
            }
            const char *env = getenv("ALBERT_METHODS");
            char *buf = new char[strlen(env)+1];
            strcpy(buf, env);
            char *p = strtok(buf, ",");
            while(p != nullptr) {
                int i = atoi(p);
                if(0 <= i && i < sizeof(include)) {
                    include[i] = true;
                }
                p = strtok(nullptr, ",");
            }
            delete[] buf;
        }

        vector<pair<double, int> > profiles[nfuncs];
        int first_func = -1;

        for (int i = 0; i < nfuncs; i++) {
            if (!include[i]) continue;

            printf("Reducing in %s mode\n", func_names[i]);

            int rank_ = 0;
            SparseMatrix SM_ = saved_SM;
            int status_ = funcs[i](SM_, cols, &rank_);

            if (first_func == -1) {
                rank = rank_;
                status = status_;
                SM = SM_;
                first_func = i;
            } else {
                if (status != status_) {
                    abort();
                }
                if (rank != rank_) {
                    abort();
                }
                if (SM != SM_) {
                    abort();
                }
            }
            profiles[i] = memory_usage;
        }
#if 0
        if (first_func >= 0) {
            for (int i = 0; i < profiles[first_func].size(); i++) {
                printf("Profile %07d:", i);
                for (int j = 0; j < nfuncs; j++) {
                    if (!include[j]) continue;
                    printf("\t%.02f/%.02f", profiles[j][i].first, profiles[j][i].second / 1024. / 1024.);
                }
                putchar('\n');
            }
        }
#endif
    }
#endif

    tt = 0;
    for (int i = 0; i < (int) SM.size(); i++) {
        tt += SM[i].size();
    }
    if (SM.size() * cols > 0) {
        printf("\t\tFill: (%.1f%% %.1fMB)->",
               p_tt / double(SM.size() * cols) * 100.,
               p_tt * sizeof(Node) / 1024. / 1024.);
        printf("(%.1f%% %.1fMB)\n",
               tt / double(SM.size() * cols) * 100.,
               tt * sizeof(Node) / 1024. / 1024.);
    }
    fflush(nullptr);

#if DEBUG_MATRIX
    PrintTheRMatrix();
#endif

/* ExtractMatrix will expand basis table & MultTable ! */
    if (status == OK) {
        status = SparseExtractFromMatrix(SM, cols, rank, n, BPtoCol);
    }
#if DEBUG_MATRIX
    PrintDependent();
#endif

    return status;
}

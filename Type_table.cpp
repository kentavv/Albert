/***  FILE :        Type_table.c                                 ***/
/***  AUTHOR:       David P Jacobs                               ***/
/***  PROGRAMMER:   Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990.                                    ***/
/***  MODULE REQUIRES:                                           ***/
/***      Type Target_type                                       ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      int CreateTypeTable()                                  ***/
/***      Type FirstTypeDegree()                                 ***/
/***      Type NextTypeSameDegree()                              ***/
/***      Basis BeginBasis()                                     ***/
/***      Basis EndBasis()                                       ***/
/***  PRIVATE ROUTINES:                                          ***/
/***      int FillTypecount()                                    ***/
/***      int InitTypetable()                                    ***/
/***      int FillTypetable()                                    ***/
/***      int InitStoreblocksizes()                              ***/
/***      int GetIndex()                                         ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines dealing with Type table. ***/
/***      Given the Target_type, generate a Type_table consisting***/
/***      of all the subtypes of the Target_type. Also generate  ***/
/***      data structures to map Degree to subtype and subtype   ***/
/***      to Type_table.                                         ***/
/***  NOTE:                                                      ***/
/***      Target_type[] is assumed to end with a 0 in the last   ***/
/***      position.                                              ***/
/*******************************************************************/

#include <vector>

using std::vector;

#include <stdio.h>
#include <stdlib.h>

#include "Type_table.h"
#include "Basis_table.h"
#include "Build_defs.h"
#include "Memory_routines.h"

static void EnterBeginBasis(int TTindex, Basis basis);
static void InitStoreblocksizes(void);
static void FillTypecount(int Cur_scan_pos);
static int InitTypetable(void);
static int FillTypetable(int Cur_scan_pos, vector<int> &Temp_dttt_index);
static int GetIndex(const Type Pntr);
static void PrintType(Type Pntr, FILE *filePtr);
#if 0
static void PrintTypetable(void);
static void PrintTypetableindex(void);
#endif

static Type Target_type = 0;                /* Input from higher level module. */
static int Target_type_len = 0;             /* Computed from Target_type.      */
static int Target_type_deg = 0;             /* Computed from Target_type.      */
static int *Type_count = NULL;              /* Used to fill Type_table.        */
static TT_node *Type_table = NULL;          /* Heart of the matter.            */
static int *Type_table_index = NULL;        /* Map type to Type_table.         */
static int Tot_subtypes = 0;                /* Computed from Type_count. Size of Type_table. */
static int *Deg_to_type_table_index = NULL; /* Map Degree to Type_table.*/
static vector<int> Store_block_sizes;       /* To find offset into Type_table. */

/*******************************************************************/
/* GLOBALS INITIALIZED:                                            */
/*     Target_type_len -- Length of the Target_type.               */
/*     Target_type_deg -- Degree of the Target_type.               */
/*     Type_count[i] -- Num. of i'th Degree subtypes in Target_type.*/
/*     Type_table[] -- Array of all the subtypes in ascending Degree.*/
/*     Type_table_index[] -- Offsets into Type_table[].             */
/*     Tot_subtypes -- Size of the Type_table.                     */ 
/*     Store_block_sizes[] -- Multiplication constants to find Index.*/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Cur_type[] -- to build the Type_table[].                    */
/* RETURNS:                                                        */
/*     1 if Type_table[] is successfully built.                    */
/*     0 otherwise.                                                */ 
/* FUNCTION:                                                       */
/*     Compute all subtypes of the given target type.              */
/*     Allocate memory for all these subtypes i.e for Type_table[].*/
/*     Initialize Type_table_index[] -- Type to Type_table.        */ 
/*     Initialize Deg_to_type_table_index[] -- Degree to Type.     */ 
/*******************************************************************/ 
int CreateTypeTable(Type Cur_type)
{
    int i = 0;
    
    Target_type_len = 0;
    while (Cur_type[i++] != 0)
        Target_type_len++;

    Target_type = GetNewType(); 
    assert_not_null(Target_type);

    for (i=0;i<Target_type_len;i++)
        Target_type[i] = Cur_type[i];

    Target_type_deg = GetDegree(Target_type); 

    InitStoreblocksizes();

    Type_count = (int *) (Mymalloc((Target_type_deg + 1) * sizeof(int)));
    assert_not_null(Type_count);

    for (i=0;i<=Target_type_deg;i++)
        Type_count[i] = 0;
    
    FillTypecount(0);

    if (InitTypetable() != OK)
        return(0);

    if(Type_count) {
      free(Type_count);
      Type_count = NULL;
    }

    return(OK);
}

int GetTargetLen(void)
{
    return(Target_type_len);
}


Type GetNewType(void)
{
    Type temp_type;
    int i;

    temp_type = (Type) (Mymalloc((Target_type_len + 1) * sizeof(Degree))); 
    assert_not_null(temp_type);

    for (i=0;i<=Target_type_len;i++)
        temp_type[i] = 0;

    return(temp_type);
}


void NameToType(Name N, Type T)
{
    assert_not_null_nv(T);

    for (int i=0;i<Target_type_len;i++)
        T[i] = Type_table[N].type[i];
}


void SubtractTypeName(Name n1, Name n2, Name *res_name)
{
    Type temp_type;
    int i;

    temp_type = GetNewType();
    assert_not_null_nv(temp_type);

    for (i=0;i<Target_type_len;i++)
        temp_type[i] = Type_table[n1].type[i] - Type_table[n2].type[i];

    *res_name = TypeToName(temp_type);

    free(temp_type);
}


int GetDegree(Type Pntr)
{
    int i, deg = 0;

    assert_not_null(Pntr);
    
    for (i=0;i<Target_type_len;i++)
        deg += Pntr[i];

    return(deg);
}


int GetDegreeName(Name n)
{
    return(GetDegree(Type_table[n].type));
}


Name TypeToName(const Type T)
{
    return(Type_table_index[GetIndex(T)]);
}


bool IsSubtype(Name n1, Name n2)
{
    for (int i=0; i<Target_type_len; i++)
        if (Type_table[n1].type[i] > Type_table[n2].type[i])
          return false;

    return true;
}


void EnterBeginBasis(int TTindex, Basis basis)
{
    Type_table[TTindex].begin_basis = basis;
}


void EnterEndBasis(int TTindex, Basis basis)
{
    Type_table[TTindex].end_basis = basis;
}

void UpdateTypeTable(Name n, Basis Begin_basis, Basis End_basis)
{
    EnterBeginBasis(n,Begin_basis);
    EnterEndBasis(n,End_basis);
}


/*******************************************************************/
/* GLOBALS FILLED:                                                 */
/*     Store_block_sizes[] -- Multiplication constants to get Index. */
/* GLOBALS REQUIRED:                                               */
/*     Target_len -- Length of the Target_type.                    */ 
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Fill the Store_block_sizes[].                               */ 
/*******************************************************************/ 
void InitStoreblocksizes(void)
{
    Store_block_sizes.resize(Target_type_len);

    Store_block_sizes[Target_type_len - 1] = 1;
    for (int i=Target_type_len - 2; i>=0; i--) {
        Store_block_sizes[i] = (Target_type[i+1] + 1) * Store_block_sizes[i+1]; 
    }
}


/*******************************************************************/
/* GLOBALS MODIFIED:                                               */
/*     Type_count[i] -- num.of subtypes of Degree i in Target_type.*/
/*     Target_type[] -- is modified on each call, but restored.    */ 
/* GLOBALS REQUIRED:                                               */
/*     Target_len -- Length of the Target_type.                    */ 
/* REQUIRES:                                                       */
/*     Cur_scan_pos -- Current scanner position in the Target_type.*/ 
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Count all subtypes of the given Target_type.                */
/*******************************************************************/ 
void FillTypecount(int Cur_scan_pos)
{
    if (Cur_scan_pos > Target_type_len) {    
        int d = 0;
        for (int i=0; i<Target_type_len; i++) {
            d += Target_type[i];
        }
        Type_count[d]++;
    } else {
        for (int i=0;i<=Target_type[Cur_scan_pos];i++) {
            int save = Target_type[Cur_scan_pos];
            Target_type[Cur_scan_pos] = i;
            FillTypecount(Cur_scan_pos + 1);
            Target_type[Cur_scan_pos] = save;
        }
    }
}


/*******************************************************************/
/* GLOBALS INITIALIZED:                                            */
/*     Type_table[] -- Array of all the subtypes in ascending Degree.*/
/*     Type_table_index[] -- Offsets into Type_table[].             */
/*     Tot_subtypes -- Size of the Type_table.                     */ 
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Type_count[i] -- Num. of i'th Degree subtypes in Target_type.*/
/* RETURNS:                                                        */
/*     1 if Type_table[] is successfully built.                    */
/*     0 otherwise.                                                */ 
/* FUNCTION:                                                       */
/*     Allocate memory for all these subtypes i.e for Type_table[].*/
/*     Initialize Type_table_index[] -- Type to Type_table.        */ 
/*     Initialize Deg_to_type_table_index[] -- Degree to Type.     */ 
/*******************************************************************/ 
int InitTypetable(void)
{
    int i;

    Tot_subtypes = 0;

    for (i=0;i<=Target_type_deg;i++)
        Tot_subtypes += Type_count[i];

    Type_table = (TT_node *) Mymalloc(Tot_subtypes * sizeof(TT_node));
    assert_not_null(Type_table);

    for (i=0;i<Tot_subtypes;i++) {
        Type_table[i].type = GetNewType(); 
        assert_not_null(Type_table[i].type);
        Type_table[i].begin_basis = Type_table[i].end_basis = 0;
    }
   
    Deg_to_type_table_index = (int *) Mymalloc((Target_type_deg + 1) * sizeof(int));
    assert_not_null(Deg_to_type_table_index);

    Deg_to_type_table_index[0] = 0;
    for (i=1;i<=Target_type_deg;i++)
        Deg_to_type_table_index[i] = Deg_to_type_table_index[i - 1] + Type_count[i - 1];

    Type_table_index = (int *) Mymalloc(Tot_subtypes * sizeof(int));
    assert_not_null(Type_table_index);
    { 
      vector<int> Temp_dttt_index(Target_type_deg + 1);         /* Used to fill Type_table_index.  */

      for (i=0; i<=Target_type_deg; i++)
          Temp_dttt_index[i] = Deg_to_type_table_index[i]; 
    
      FillTypetable(0, Temp_dttt_index);
    }

    return OK;
}


void DestroyTypeTable(void)
{
    if(Type_table) {
      for(int i=0;i<Tot_subtypes;i++) {
        if(Type_table[i].type) free(Type_table[i].type);
      }

      free(Type_table);
      Type_table = NULL;
    }

    if(Deg_to_type_table_index) {
      free(Deg_to_type_table_index);
      Deg_to_type_table_index = NULL;
    }

    if(Type_table_index) {
      free(Type_table_index);
      Type_table_index = NULL;
    }

    Store_block_sizes.clear();
}



/*******************************************************************/
/* GLOBALS FILLED:                                                 */
/*     Type_table[] -- All the subtypes.                           */
/*     Type_table_index[] -- Offsets into Type_table.              */
/* GLOBALS REQUIRED:                                               */
/*     Target_len -- Length of the Target_type.                    */ 
/* GLOBALS MODIFIED:                                               */
/*     Temp_dtttx_index[] -- Where to place the next subtype.      */
/* REQUIRES:                                                       */
/*     Cur_scan_pos -- Current scanner position in the Target_type.*/ 
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Fill the Type_table.                                        */ 
/*******************************************************************/ 
int FillTypetable(int Cur_scan_pos, vector<int> &Temp_dttt_index)
{
    if (Cur_scan_pos > Target_type_len) {    
        int d = 0;
        for (int i=0;i<Target_type_len;i++)
            d += Target_type[i];

        for (int i=0;i<=Target_type_len;i++)
            Type_table[Temp_dttt_index[d]].type[i] = Target_type[i];

        Type_table_index[GetIndex(Target_type)] = Temp_dttt_index[d];
        Temp_dttt_index[d]++;
    } else {
        for (int i=0;i<=Target_type[Cur_scan_pos];i++) {
            int save = Target_type[Cur_scan_pos];
            Target_type[Cur_scan_pos] = i;
            FillTypetable(Cur_scan_pos + 1, Temp_dttt_index);
            Target_type[Cur_scan_pos] = save;
        }
    }
    return(OK);
}


/*******************************************************************/
/* GLOBALS REQUIRED:                                               */
/*     Target_len -- Length of the Target_type.                    */ 
/*     Store_block_sizes[] -- Multiplication constants to get Index. */
/* REQUIRES:                                                       */
/*     Type for which index into Type_table[] is to be found.      */
/* RETURNS:                                                        */
/*     Index into the Type_table.                                  */
/*******************************************************************/ 
int GetIndex(const Type Pntr)
{
    assert_not_null(Pntr);

    int result = 0;

    for (int i=0;i<Target_type_len;i++) 
        result += Pntr[i] * Store_block_sizes[i];

    return(result);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     D -- Degree.                                                */ 
/* RETURNS:                                                        */
/*     pointer to first subtype in Type_table[].                   */ 
/*******************************************************************/ 
Name FirstTypeDegree(Degree D)
{
    if ((D >= 0) && (D <= Target_type_deg))
        return(Deg_to_type_table_index[(int)D]);
    else
        return(-1);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Pntr -- pointer to a subtype in a TT_node in Type_table[].  */ 
/* RETURNS:                                                        */
/*     pointer to next subtype in TT_node in Type_table[].         */ 
/*******************************************************************/ 
Name NextTypeSameDegree(Name n)
{
    if (n >= (Tot_subtypes - 1))
        return(-1);

    if (GetDegreeName(n + 1) > GetDegreeName(n))
        return(-1);

    return(n + 1);
}


Basis FirstBasis(Name N)
{
    return(BeginBasis(N));
}


Basis NextBasisSameType(Basis B)
{
    int Nextbasistobefilled = GetNextBasisTobeFilled();

    if ((B+1) >= Nextbasistobefilled) 		/* Check range of B,B+1 */
        return(0);

    if (GetType(B) != GetType(B+1))			/* B,B+1 valid basis numbers */
        return(0);							/* but have different types */
    else
        return(B+1);						/* Have same types */
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Pntr -- pointer to a subtype in a TT_node in Type_table[].  */ 
/* RETURNS:                                                        */
/*     First Basis element with the given type.                    */ 
/*******************************************************************/ 
Basis BeginBasis(Name n)
{
    return(Type_table[n].begin_basis);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Pntr -- pointer to a TT_node in Type_table[].               */ 
/* RETURNS:                                                        */
/*     Last Basis element with the given type.                     */ 
/*******************************************************************/ 
Basis EndBasis(Name n)
{
    return(Type_table[n].end_basis);
}

void PrintType(Type Pntr, FILE *filePtr)
{
    int i;

    assert_not_null_nv(Pntr);

    for (i=0;i<Target_type_len;i++)
        fprintf(filePtr, "%d",Pntr[i]);
}


void PrintTypeName(Name n, FILE *filePtr)
{
    PrintType(Type_table[n].type, filePtr);
}


#if 0
void PrintTypetable(void)
{
    int i,j;

    printf("Type Table: \n");
    for (i=0;i<Tot_subtypes;i++) {
        printf("   ");
        for (j=0;j<Target_type_len;j++)
            printf("%d",Type_table[i].type[j]);
        printf(" %3d %3d",Type_table[i].begin_basis,Type_table[i].end_basis);
        printf("\n");
    }
}


void PrintTypetableindex(void)
{
    int i,j;

    printf("Type table index entries are :\n");
    for (i=0;i<Tot_subtypes;i++)
        printf("%d\n",Type_table_index[i]);
}
#endif


/*******************************************************************/
/***  FILE :     Po_create_poly.c                                ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      struct polynomial *Create_poly()                       ***/
/***  PRIVATE ROUTINES:                                          ***/
/***      void Print_tree()                                      ***/
/***      void Print_opr_prod_tree()                             ***/
/***      void Print_art_word()                                  ***/
/***      void Create_exp_str()                                  ***/
/***      int Found_minus()                                      ***/
/***      void Free_tnode_tree()                                 ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines needed to create a       ***/
/***      polynomial,from the input string entered by user.      ***/
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "Po_create_poly.h"	
#include "Po_create_poly_pri.h"	
#include "Po_expand_poly.h"
#include "Memory_routines.h"
#include "Po_parse_poly.h"	
#include "Po_semantics.h"	
#include "Po_prod_bst.h"	
#include "Po_parse_exptext.h"	
#include "Strings.h"

static void Print_tree(struct unexp_tnode *Pntr);
static void Print_opr_prod_tree(struct unexp_tnode *Pntr);
static void Print_art_word(struct unexp_tnode *Pntr);
static void Create_exp_str(struct unexp_tnode *Pntr, char **Str_ptr, int  *Maxsize_str_ptr);
static int Found_minus(struct unexp_tnode *Pntr);
static void Free_tnode_tree(struct unexp_tnode *Pntr);

extern jmp_buf env;

/*******************************************************************/
/* Called from Main() in driver.c.                                 */
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     In_str -- polynomial string, operand portion of the         */
/*               commands "i [polynomial]" and "q [polynomial]".   */ 
/* RETURNS:                                                        */
/*     Pointer to the created polynomial if In_str is successfully */
/*         parsed.                                                 */
/*     NULL otherwise.                                             */
/* FUNCTION:                                                       */
/*     Creates the structure polynomial through a series of calls  */
/*     to routines in files Po_parse_poly.c, Po_expand_poly.c and  */
/*     Po_parse_exptext.c                                          */ 
/*     First call Create_parse_tree() with In_str to get the parse */
/*     tree corresponding to In_str.                               */
/*     Then expand the parse tree by calling Expand_parse_tree().  */ 
/*     Then get the simplified parse tree having basic operators   */
/*     like JUXT_PROD, SCALAR_MULT along with small letters and    */
/*     scalars at the leaves by calling Simplify_parse_tree(),     */
/*     in a loop until no further simplification is possible.      */
/*     The Simplified tree is translated into a string by          */
/*     calling Create_exp_str() which traverses the tree in        */
/*     inorder.                                                    */
/*     Finally the expanded string is parsed by calling            */
/*     Parse_exptext(), which returns a pointer to the polynomial  */
/*     which in turn returned to the caller of Create_poly().      */ 
/* NOTE:                                                           */
/*     The intermediate trees created during the creation of       */
/*     polynomial can be seen by switching on the DEBUG_EXP flag.  */
/*******************************************************************/ 
struct polynomial *Create_poly(char In_str[], int *In_str_len, int *Out_of_memory)
{
    /*static struct unexp_tnode *temp;*/

    struct unexp_tnode *Unexp_tree1;
    struct unexp_tnode *Unexp_tree2;
    struct unexp_tnode *Exp_tree1;
    struct unexp_tnode *Exp_tree2;
    struct unexp_tnode *Exp_tree3;
    struct unexp_tnode *Exp_tree4;
    struct polynomial *poly;

    char   *exp_poly_str;
    static char   **Exp_poly_str_ptr; /* Exp_poly_str may be expanded! */
    static int    Maxsize_exp_poly_str;
    int Modified = FALSE; /* Has tree been changed in this call? */
#if DEBUG_EXP
    int count; /* Debugging information. No of calls to Simplify */
#endif
    /*int i;*/
    int first_time = TRUE;


/* Allocate space for exp_poly_str first_time */
    if (first_time) {
        exp_poly_str = (char*) Mymalloc(EXP_STR_LEN);
        Maxsize_exp_poly_str = EXP_STR_LEN;
        first_time = FALSE;
    }
    exp_poly_str[0] = '\0';
    Exp_poly_str_ptr = &exp_poly_str;

    Unexp_tree1 = NULL;
    Unexp_tree2 = NULL;
    Exp_tree1 = NULL;
    Exp_tree2 = NULL;
    Exp_tree3 = NULL;
    Exp_tree4 = NULL;

    if (setjmp(env) != 0) {
        Free_tnode_tree(Unexp_tree1);
        Free_tnode_tree(Unexp_tree2);
        Free_tnode_tree(Exp_tree1);
        Free_tnode_tree(Exp_tree2);
        Free_tnode_tree(Exp_tree3);
        Free_tnode_tree(Exp_tree4);
        *Out_of_memory = TRUE;
        printf("Command unsuccessfull.\n");
        return(NULL);
    }

/* Parse tree for the polynomial string In_str entered by user */
    Unexp_tree1 = Create_parse_tree(In_str);

    if (Unexp_tree1 == NULL)
        return(NULL); 

#if DEBUG_EXP
    Print_tree(Unexp_tree1);
    printf("\n");
#endif

/* Expand the Parse tree by using formulae for operators */
    Unexp_tree2 = Expand_parse_tree(Unexp_tree1); 

#if DEBUG_EXP
    Print_tree(Unexp_tree2);
    printf("\n");
#endif

/* Simplify the Expanded tree i.e apply distributive laws. */
    Exp_tree1 = Simplify_parse_tree(Unexp_tree2,&Modified); 

#if DEBUG_EXP
    count = 1;
    printf("simplified tree %d\n",count);
    Print_tree(Exp_tree1);
    printf("\n");
#endif

    while (Modified) {
        Modified = FALSE;
        Exp_tree2 = Simplify_parse_tree(Exp_tree1,&Modified); 

#if DEBUG_EXP
        count++;
        printf("count = %d Modified = %d\n",count,Modified);
        printf("simplified tree\n");
        Print_tree(Exp_tree2);
        printf("\n");
        printf("freeing Exp_tree1\n");
#endif

        Free_tnode_tree(Exp_tree1);
        Exp_tree1 = Exp_tree2;
        Exp_tree2 = NULL;

    }


/* No more simplifications needed! */
    Modified = FALSE;

/* Get rid of Unary minus by multiplying that subtree by -1. */ 
    Exp_tree3 = Elim_subtraction(Exp_tree1,&Modified); 

#if DEBUG_EXP
    count = 1;
    Print_tree(Exp_tree3);
    printf("\n");
#endif

    while (Modified) {
        Modified = FALSE;
        Exp_tree4 = Elim_subtraction(Exp_tree3,&Modified); 

#if DEBUG_EXP
        count++;
        printf("count = %d Modified = %d\n",count,Modified);
        Print_tree(Exp_tree4);
        printf("\n");
        printf("freeing Exp_tree3\n");
#endif

        Free_tnode_tree(Exp_tree3);
        Exp_tree3 = Exp_tree4;
        Exp_tree4 = NULL;

    }

/* Traverse the tree in inorder and write it as a string */
    Create_exp_str(Exp_tree3,Exp_poly_str_ptr,&Maxsize_exp_poly_str);
    Free_tnode_tree(Unexp_tree1);
    Free_tnode_tree(Unexp_tree2);
    Free_tnode_tree(Exp_tree1);
    Free_tnode_tree(Exp_tree3);

#if DEBUG_EXP
    printf("%s",exp_poly_str);
    printf("\n");
#endif


/* Parse the string to create the polynomial. */
    poly = Parse_exptext(*Exp_poly_str_ptr);

    *In_str_len = strlen(exp_poly_str);
    free(exp_poly_str);
    return(poly);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Pntr -- root of the tree of tnodes                          */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Print the tree in preorder.                                 */ 
/* NOTE:                                                           */
/*     Called only when DEBUG_EXP flag is on.                      */ 
/*     Very useful to see how the parse tree gets simplified.      */
/*******************************************************************/ 
void Print_tree(struct unexp_tnode *Pntr)
{
    if (Pntr != NULL) {
        if ((Pntr->op != SMALL_LETTER) && (Pntr->op != SCALAR) &&
            (Pntr->op != OPERATOR_PROD) && (Pntr->op != ARTIFICIAL_WORD)) {
            printf("%c(",OPR_SYMBOL[Pntr->op]);
            Print_tree(Pntr->operand1);
            if (Pntr->operand2 != NULL) {
                printf(",");
                Print_tree(Pntr->operand2);
                if (Pntr->operand3 != NULL) {
                    printf(",");
                    Print_tree(Pntr->operand3);
                }
            }
            printf(")");
        }
        else if (Pntr->op == SMALL_LETTER)
            printf("%c",Pntr->s_letter);
        else if (Pntr->op == SCALAR)
            printf("%d",Pntr->scalar_num);
        else if (Pntr->op == OPERATOR_PROD) {
            printf("%c(",OPR_SYMBOL[Pntr->op]);
            Print_tree(Pntr->operand1);
            Print_opr_prod_tree(Pntr->operand2);
            printf(")");
        }
        else if (Pntr->op == ARTIFICIAL_WORD) {
            printf("%c(",OPR_SYMBOL[Pntr->op]);
            Print_tree(Pntr->operand1);
            Print_art_word(Pntr->operand2);
            printf(")");
        }

    }
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Pntr -- Pointer to a list of Operators (left, right) and    */
/*             their operands.                                     */  
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Handle the special case of printing Operators.              */
/*     Print all operators in the list and their operands.         */ 
/* NOTE:                                                           */
/*     Called only by Print_tree(). i.e Called only for debugging. */
/*******************************************************************/ 
void Print_opr_prod_tree(struct unexp_tnode *Pntr)
{
    while (Pntr != NULL) {
        printf(",");
        Print_tree(Pntr);
        Pntr = Pntr->next;
    }
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Pntr -- Pointer to a tree corresponding to operator         */
/*             ARTIFICIAL_WORD.                                    */ 
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Handle the special case of printing ARTIFICIAL_WORD tree.   */ 
/*     Print the tree corresponding to operator ARTIFICIAL_WORD.   */ 
/* NOTE:                                                           */
/*     Called only by Print_tree(). i.e Called only for debugging. */
/*******************************************************************/ 
void Print_art_word(struct unexp_tnode *Pntr)
{
    while (Pntr != NULL) {
        printf(",");
        Print_tree(Pntr);
        Pntr = Pntr->next;
    }
}


/*******************************************************************/
/* MODIFIES:                                                       */
/*     Str_ptr -- Pointer to the string.                           */
/*     Maxsize_ptr -- Maxsize of the string pointed by Str_ptr.    */
/* REQUIRES:                                                       */
/*     Pntr -- pointer to the root of the expanded polynomial tree.*/
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Build the string pointed by Str_ptr, for the given tree, by */
/*     traversing it in inorder.                                   */
/* NOTE:                                                           */
/*     Str_cat() is used for string concatenation. This special    */
/*     function is needed because when size of the string pointed  */
/*     by Str_ptr may not be enough to be concatenated with        */
/*     another string. In which case Str_cat() reallocates more    */
/*     space for the string pointed by Str_ptr and changes the     */
/*     Max_size to reflect the change in string size.              */
/*     Thus string grows varies dynamically to accommodate large   */
/*     polynomials.                                                */
/*******************************************************************/ 
void Create_exp_str(struct unexp_tnode *Pntr, char **Str_ptr, int  *Maxsize_str_ptr)
{
    char Temp_str[10];

    if (Pntr != NULL) {

        if ((Pntr->op != SMALL_LETTER) && (Pntr->op != SCALAR)) {

            if (Pntr->op == JUXT_PROD) {
                sprintf(Temp_str,"(");
                Str_cat(Str_ptr,Temp_str,Maxsize_str_ptr);
            }

            Create_exp_str(Pntr->operand1,Str_ptr,Maxsize_str_ptr);

            if (Pntr->op == ADDITION) {
                if (!Found_minus(Pntr->operand2)) {
                    sprintf(Temp_str,"+");
                    Str_cat(Str_ptr,Temp_str,Maxsize_str_ptr);
                }
            }
            else if (Pntr->op == SUBTRACTION) {
                sprintf(Temp_str,"-");
                Str_cat(Str_ptr,Temp_str,Maxsize_str_ptr);
            }

            Create_exp_str(Pntr->operand2,Str_ptr,Maxsize_str_ptr);

            if (Pntr->op == JUXT_PROD) {
                sprintf(Temp_str,")");
                Str_cat(Str_ptr,Temp_str,Maxsize_str_ptr);
            }
        }

        else if (Pntr->op == SMALL_LETTER) {
                sprintf(Temp_str,"%c",Pntr->s_letter);
                Str_cat(Str_ptr,Temp_str,Maxsize_str_ptr);
            }
        else if (Pntr->op == SCALAR) {
            if (Pntr->scalar_num == -1) {
                sprintf(Temp_str,"-");
                Str_cat(Str_ptr,Temp_str,Maxsize_str_ptr);
            }
            else if (Pntr->scalar_num != 1) {
                sprintf(Temp_str,"%d",Pntr->scalar_num);
                Str_cat(Str_ptr,Temp_str,Maxsize_str_ptr);
            }
        }
    }
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Pntr -- Pointer to a tree.                                  */ 
/* RETURNS:                                                        */
/*     1 if first if any of the operand1's of the tree are scalars */
/*          less than 0.                                           */
/*     0  otherwise.                                               */ 
/* FUNCTION:                                                       */
/*     Since the tree is traversed in inorder, avoid printing      */
/*     something like x+-4y. i.e + needs to be suppressed in case  */
/*     of scalars less than 0, thus correctly prints x-4y.         */ 
/*******************************************************************/ 
int Found_minus(struct unexp_tnode *Pntr)
{
    if (Pntr == NULL)
        return(0);
    else if (Pntr->scalar_num < 0)
        return(1);
    else return(Found_minus(Pntr->operand1));
}


/*******************************************************************/
/* MODIFIES:                                                       */
/*     Pntr -- Pointer to a tree.                                  */ 
/* REQUIRES: None.                                                 */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Free all the space used for the tree with root pointer as   */
/*     Pntr.                                                       */
/* NOTE:                                                           */
/*     Traveses the tree in inorder to free all the nodes.         */
/*     The nodes are freed and entered onto Free tnode queue       */
/*     through a call to Free_tnode().                             */  
/*******************************************************************/ 
/*
 * Input   : pointer to the root of the unexpanded polynomial tree.
 * Function: free the memory space by going to each leaf node. 
 *
 */
void Free_tnode_tree(struct unexp_tnode *Pntr)
{
    /*static int i = 1;*/

    if (Pntr != NULL) {
        if ((Pntr->operand1 == NULL) && (Pntr->operand2 == NULL) &&
            (Pntr->operand3 == NULL) && (Pntr->next == NULL))
            Free_tnode(Pntr);
        else {
            Free_tnode_tree(Pntr->operand1);
            Free_tnode_tree(Pntr->operand2);
            Free_tnode_tree(Pntr->operand3);
            Free_tnode_tree(Pntr->next);
            Free_tnode(Pntr);
        }
    }
}

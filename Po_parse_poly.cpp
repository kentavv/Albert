/*******************************************************************/
/***  FILE :     Po_parse_poly.c                                 ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      struct unexp_tnode *Create_parse_tree()                ***/ 
/***      void Out_of_int_bounds()                               ***/ 
/***  PRIVATE ROUTINES:                                          ***/
/***      static void Init_prod_tree()                           ***/
/***      static int Reduce()                                    ***/
/***      static int Get_next_token()                            ***/
/***      static void Itoa()                                     ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines used to parse a          ***/
/***      polynomial string.                                     ***/ 
/***  NOTE:                                                      ***/
/***      Simple precedence parsing technique is used.           ***/
/*******************************************************************/


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Po_parse_poly.h"
#include "Po_parse_poly_pri.h"
#include "Po_syn_stack.h"
#include "Po_semantics.h"
#include "Po_prod_bst.h"

static void Init_prod_tree(PROD_TREEPTR *Prod_bst_root_ptr);
static void Handle_char_pair_error(int *Syntax_error);
static void Handle_reducibility_error(int *Syntax_error);
static void Handle_stackability_error(int *Syntax_error);
static int Reduce(int Syn_stack[], int *Sp_ptr, PROD_TREEPTR Prod_bst_root, struct unexp_tnode *Sem_stack[]);
static int Get_next_token(char Poly_str[], int *Poly_str_indx_ptr, char *Current_letter_ptr, int  *Current_int_ptr);
static int Out_of_int_bounds(int i);
static void Itoa(int N, char S[]);

/*******************************************************************/
/* MODIFIES:                                                       */
/*     None.                                                       */
/* REQUIRES:                                                       */
/*     Poly_str -- Polynomial string to be parsed.                 */
/* RETURNS:                                                        */
/*     Pointer to the parse tree if parsing is successfull.        */
/*     NULL otherwise.                                             */
/* FUNCTION:                                                       */
/*     Parses the string Poly_str looking for syntax or semantic   */
/*     errors, and builds the Parse tree simultaneously.           */ 
/* NOTE:                                                           */
/*     Parsing action can be observed by changing  DEBUG_PARSE     */
/*     flag to 1 in Po_parse_poly.h.                               */
/*******************************************************************/ 
struct unexp_tnode *Create_parse_tree(char Poly_str[])
{
    int  Syn_stack[STACK_SIZE];           /* stack used while parsing */
    struct unexp_tnode *Sem_stack[STACK_SIZE];  
    static PROD_TREEPTR Prod_bst_root = NULL;
    int  sp = -1;				

    int  token;                            /* token number returned */
    int  Poly_str_indx = 0;

    int  Syntax_error = FALSE;

    int  Current_int=0;                    /* hold the integer when lex finds */
    char Current_letter = ' ';             /* hold the letter when lex finds */

    int  i;
    static int first_time = TRUE; 

/* Store the productions into binary search tree if running for the first time */
    if (first_time) {
        Init_prod_tree(&Prod_bst_root);
        first_time = FALSE;
    }
    
    for (i=0;i<STACK_SIZE;i++)
        Sem_stack[i] = NULL;
    Push_token(Syn_stack,&sp,CENT);

    token = Get_next_token(Poly_str,&Poly_str_indx,&Current_letter,&Current_int) ;

    while ((token > 0) && (!Syntax_error)) {

#if DEBUG_PARSE
        Print_syn_stack(Syn_stack,sp);
        printf("token = %d\n",token);
#endif

        if ( (Sp_relation[Get_top_token(Syn_stack,sp)][token] == LT_RELATION) ||
               (Sp_relation[Get_top_token(Syn_stack,sp)][token] == EQ_RELATION) )  {
            Push_token(Syn_stack,&sp,token);
            if((token == LETTER) || (token == INT)) {
                Store_semantics(Sem_stack,sp,token,Current_letter,Current_int);
                Current_letter = ' ';
                Current_int = 0;
            }
            token = Get_next_token(Poly_str,&Poly_str_indx,&Current_letter,&Current_int) ;
        } 
        else {
            if ((sp == 1) && (Get_top_token(Syn_stack,sp) == 1) && (!Syntax_error) && (token == DOLLAR) ) {

#if DEBUG_PARSE
                printf(" VALID STRING\n");
#endif

                return(Sem_stack[1]);
                break;
            }
            else if (Sp_relation[Get_top_token(Syn_stack,sp)][token] == NO_RELATION)
                Handle_char_pair_error(&Syntax_error);
            else if (Sp_relation[Get_top_token(Syn_stack,sp)][token] == GT_RELATION){
                if (!Reduce(Syn_stack,&sp,Prod_bst_root,Sem_stack))
                    Handle_reducibility_error(&Syntax_error);
                else if ((Sp_relation[Get_next_to_top_token(Syn_stack,sp)][Get_top_token(Syn_stack,sp)] == NO_RELATION) ||
                (Sp_relation[Get_next_to_top_token(Syn_stack,sp)][Get_top_token(Syn_stack,sp)] == GT_RELATION))
                Handle_stackability_error(&Syntax_error);
            }
        }
    }
    return(NULL);
}


/*******************************************************************/
/* MODIFIES:                                                       */
/*     Prod_bst_root_ptr -- root of Binary search tree of          */
/*                          productions                            */
/* REQUIRES:                                                       */
/*     List of productions. (declared in Po_create_poly.h)         */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Insert all the 36 productions in the tree.                  */
/* NOTE:                                                           */
/*     This routine is called only once when system comes up.      */ 
/*******************************************************************/ 
void Init_prod_tree(PROD_TREEPTR *Prod_bst_root_ptr)
{
    int  i;
    for (i=0; i<NUM_PRODS; i++)
        Prod_insert(Prod_nodes[i].rhs,Prod_nodes[i].lhs,i+1,Prod_bst_root_ptr);
}


/*******************************************************************/
/* MODIFIES:                                                       */
/*     *Syntax_error                                               */
/* REQUIRES: None.                                                 */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Issue an error message.                                     */
/*******************************************************************/ 
void Handle_char_pair_error(int *Syntax_error)
{

#if DEBUG_PARSE
    printf("**** Character pair error\n");
    printf(" INVALID STRING\n");
#endif

    printf("**** syntax error\n");
    *Syntax_error = TRUE;
}

void Handle_reducibility_error(int *Syntax_error)
{

#if DEBUG_PARSE
    printf("**** Reducibility error\n");
    printf(" INVALID STRING\n");
#endif

    printf("**** syntax error\n");
    *Syntax_error = TRUE;
}

void Handle_stackability_error(int *Syntax_error)
{

#if DEBUG_PARSE
    printf("**** Stackability error\n");
    printf(" INVALID STRING\n");
#endif

    printf("**** syntax error\n");
    *Syntax_error = TRUE;
}


/*******************************************************************/
/* MODIFIES:                                                       */
/*     Syn_stack -- Syntax stack, to push and pop tokens.          */
/*     *Sp_ptr -- stack pointer.                                   */
/*     Sem_stack -- Semantic stack, to push and pop tnode pointers.*/
/* REQUIRES:                                                       */
/*     Prod_bst_root -- root of the tree of productions.           */
/* RETURNS:                                                        */
/*     1 if succeeds in reduction.                                 */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Finds the Handle and reduces it. Pushes the lhs of the      */
/*     production onto the syntax stack. Also does the necessary   */
/*     semantic action.                                            */ 
/* NOTE:                                                           */
/*     Reductions can be observed by changing  DEBUG_PARSE flag to */
/*     1 in Po_parse_poly.h.                                       */
/*******************************************************************/ 
/*
 * Input   : simple precedence matrix, stack(syntax and semantic),
 *           address of the stack pointer (sp changes) and the root
 *           of the tree of productions.
 * Function: returns TRUE if there is a successfull reductions pushing the
 *           lhs of the production and does the semantic action.
 *           returns FALSE if there is an error while trying to reduce.
 *        
 */
int Reduce(int Syn_stack[], int *Sp_ptr, PROD_TREEPTR Prod_bst_root, struct unexp_tnode *Sem_stack[])
{
    PROD_TREEPTR  prod_index;

    int  sem_sp;

    int  temp_token;
    char temp_str[HANDLE_LEN];			/* Hold the handle until reduction */
    char temp1_str[HANDLE_LEN];		/* Helps creating the handle */ 
    temp_str[0] = '\0';

    temp_token = Get_top_token(Syn_stack,*Sp_ptr);
    Itoa(temp_token,temp_str);

    sem_sp = *Sp_ptr;

    while (Sp_relation[Get_next_to_top_token(Syn_stack,*Sp_ptr)][Get_top_token(Syn_stack,*Sp_ptr)] == EQ_RELATION){
        Pop_token(Syn_stack,Sp_ptr);
        temp_token = Get_top_token(Syn_stack,*Sp_ptr);
        Itoa(temp_token, temp1_str);
        strcat(temp1_str, temp_str);
        strcpy(temp_str, temp1_str);
    }

    if ((prod_index = Prod_member(temp_str, Prod_bst_root)) == NULL)
        return(0);
    else {
        Pop_token(Syn_stack,Sp_ptr);

#if DEBUG_PARSE
        printf("reduction number = %d",prod_index->prodnum);
#endif

        Reduce_semantics(prod_index->prodnum,Sem_stack,&sem_sp);

        /*  REDUCTION  */
        Push_token(Syn_stack,Sp_ptr,prod_index->lhs);
        return(1);
    }
}


/*******************************************************************/
/* MODIFIES:                                                       */
/*     *Poly_str_indx_ptr -- Place in Poly_str where scanner is.   */ 
/*     *Cur_letter_ptr -- changes it if the token is small letter. */ 
/*     *Cur_int_ptr -- changes it if the token an integer.         */ 
/* REQUIRES:                                                       */
/*     Poly_str -- String in which to find the next token.         */
/* RETURNS:                                                        */
/*     Token if valid token                                        */
/*     INVALID_TOKEN otherwise.                                    */
/* FUNCTION:                                                       */
/*     Scans the Poly_str from the current position for token, and */
/*     moves the scanner by incrementing *Poly_str_indx_ptr.       */
/* NOTE:                                                           */
/*     If the token is an integer, stores it in Current_int and if */
/*     the token is a small letter, stores it in Current_letter.   */
/*******************************************************************/ 
int Get_next_token(char Poly_str[], int *Poly_str_indx_ptr, char *Current_letter_ptr, int  *Current_int_ptr)
{
    int i;

    if (*Poly_str_indx_ptr < (int)strlen(Poly_str)) {
		
        switch(Poly_str[(*Poly_str_indx_ptr)++]) {

            case '+':
                return(PLUS);
                break;

            case '-':
                return(MINUS);
                break;

            case '^':
                return(EXP_SYM);
                break;

            case '*':
                return(STAR);
                break;

            case '(':
                return(LEFT_PARAN);
                break;

            case ')':
                return(RIGHT_PARAN);
                break;
            
            case '[':
                return(LEFT_BRACKET);
                break;

            case ',':
                return(COMMA);
                break;

            case ']':
                return(RIGHT_BRACKET);
                break;

            case 'J':
                return(JORDAN);
                break;

            case '<':
                return(TOKEN_LESS);
                break;

            case '>':
                return(TOKEN_GREATER);
                break;


            case '{':
                return(LEFT_BRACE);
                break;

            case '}':
                return(RIGHT_BRACE);
                break;

            case '`':
                return(LEFT_QUOTE);
                break;

            case '\'':
                return(RIGHT_QUOTE);
                break;

            case ';':
                return(SEMI_COLON);
                break;

            case ':':
                return(COLON);
                break;

            case 'W':
                return(A_WORD);
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                i = *Poly_str_indx_ptr  - 1;
                while (i<(int)strlen(Poly_str) && isdigit(Poly_str[i])) 
                    *Current_int_ptr = (*Current_int_ptr * 10) + (Poly_str[i++] - '0');
                *Poly_str_indx_ptr  = i;
                if ((*Current_int_ptr) == 0) {
                    printf("Coefficient should be greater than 0\n");
                    return(INVALID_TOKEN);
                    break;
                }
                if (Out_of_int_bounds(*Current_int_ptr)) {
                    return(INVALID_TOKEN);
                    break;
                }
                return(INT);
                break;

            default:
                if (islower(Poly_str[*Poly_str_indx_ptr - 1])) {
                    *Current_letter_ptr = Poly_str[*Poly_str_indx_ptr - 1];
                    return(LETTER);
                    break;
                }
                else if (isupper(Poly_str[*Poly_str_indx_ptr - 1])) {
                     printf("Upper case letters not allowed in polynomial.\n");
                    return(INVALID_TOKEN);
                    break;
                }
                else {
                    printf("Illegal character in polynomial\n");
                    return(INVALID_TOKEN);
                    break;
                }
         }
    }
    else
        return(DOLLAR);
}
				 
/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     i -- An integer whose bound is to be checked.               */ 
/* RETURNS:                                                        */
/*     1 if i is out of bounds.                                    */
/*     0 otherwise.                                                */ 
/* FUNCTION:                                                       */
/*     Check if "i" is greater than MAX_INT. If yes, then return 1 */
/*     else return 0.                                              */
/*******************************************************************/ 
int Out_of_int_bounds(int i)
{
    if ((i>=MAX_INT) || (i<=0)) {
        printf("Scalar is out of bounds.\n");
        return(1);
    }
    else
        return(0);
}

void Itoa(int N, char S[])
{
    sprintf(S, "%d", N);
}

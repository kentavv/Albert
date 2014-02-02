/*******************************************************************/
/***  FILE :     Po_semantics.c                                  ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      void Reduce_semantics()                                ***/
/***      void Store_semantics()                                 ***/
/***      void Print_sem_stack()                                 ***/
/***  PRIVATE ROUTINES:                                          ***/
/***      void Push_tnode_ptr()                                  ***/
/***      void Pop_tnode_ptr()                                   ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines for doing Semantics      ***/ 
/***      while parsing the polynomial string.                   ***/
/***      These routines are called from Po_parse_poly.c         ***/
/*******************************************************************/

#include	<stdio.h>
#include	"Po_parse_poly.h"
#include	"Po_syn_stack.h"
#include	"Po_semantics.h"


/*******************************************************************/
/* MODIFIES:                                                       */
/*     Sem_stack -- Semantic stack, stack of pointers to tnodes.   */ 
/*     *Sem_sp_ptr -- Semantic stack pointer.                      */
/* REQUIRES:                                                       */
/*     Reduction_num -- Reduction number.                          */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     1.Semantic stack manipulation:                              */
/*        Manipulates the semantic stack by pushing and/or popping */
/*        tnodes, depending on Reduction number.                   */
/*     2.Generation of partial tree:                               */
/* NOTE:                                                           */
/*     There is a 1-1 correspondence between syntax and semantic   */
/*     stack.                                                      */
/*******************************************************************/ 
void Reduce_semantics(Reduction_num,Sem_stack,Sem_sp_ptr)
int  Reduction_num;
struct unexp_tnode *Sem_stack[];
int *Sem_sp_ptr;
{
    int i;
    struct unexp_tnode *Temp_tnode_ptr;	
    struct unexp_tnode *temp_tnode_ptr1;	
    struct unexp_tnode *temp_tnode_ptr2;	
    struct unexp_tnode *trash;	
    struct unexp_tnode *Unexp_tnode_alloc();
    struct unexp_tnode *Pop_tnode_ptr();
    void   Push_tnode_ptr();

    switch (Reduction_num) {

        case   1  : /* polynomial -> term */
            break;

        case  2   : /* polynomial -> - term */
            Temp_tnode_ptr = Unexp_tnode_alloc(); 
            Temp_tnode_ptr->operator = UNARY_MINUS;
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case  3   : /* polynomial -> + term */ 
            Temp_tnode_ptr = Unexp_tnode_alloc(); 
            Temp_tnode_ptr->operator = UNARY_PLUS;
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case   4  : /* polynomial -> polynomial + term */ 
            Temp_tnode_ptr = Unexp_tnode_alloc(); 
            Temp_tnode_ptr->operator = ADDITION;
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case   5  : /* polynomial -> polynomial - term */ 
            Temp_tnode_ptr =  Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = SUBTRACTION;
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case   6  : /* term -> product */
            break;
            
        case   7  : /* term -> int product */ 
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = SCALAR_MULT;
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;
            
        case   8  : /* product -> atom_or_dbl_atom */
            break;

        case   9  : /* product -> atom_or_dbl_atom ^ int */ 
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = EXPONENTIATION;
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case   10 : /* atom_or_dbl_atom -> atom_or_dbl_atom  * atom_or_dbl_atom */
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = JORDAN_PROD;
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case   11 : /* atom_or_dbl_atom -> ratom */
            break;

        case  12  : /* atom_or_dbl_atom -> double_atom */
            break;

        case  13  : /* double_atom -> atom ratom */
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = JUXT_PROD;
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case  14  : /* ratom -> atom */
        case  15  : /* atom -> small_letter */
        case  16  : /* atom -> commutator */
        case  17  : /* atom -> associator */
        case  18  : /* atom -> jacobi */
        case  19  : /* atom -> jordan_associator */
        case  20  : /* atom -> operator_product */
        case  21  : /* atom -> artificial_word */
            break;
        case  22  : /* atom -> ( polynomial- ) */
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr =  Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case  23  : /* commutator -> [ polynomial-, polynomial- ] */ 
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = COMMUTATION;
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;
			
        case  24  : /* associator -> ( polynomial-, polynomial-, polynomial- )  */
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = ASSOCIATION;
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand3 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;
			
        case  25  : /* jacobi -> J(polynomial-,polynomial-,polynomial-) */ 
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = JACOBI;
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand3 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case  26  : /* jordan_associator -> <polynomial-,polynomial-,polynomial-> */
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = JORDAN_ASSOC;
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand3 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;
			
        case  27  : /* polynomial- -> polynomial */
            break;

        case  28  : /* operator_product -> { atom operator_list- } */
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = OPERATOR_PROD;
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case  29  : /* operator_list- -> operator_list */
        case  30  : /* operator_list -> operator */
            break;

        case  31  : /* operator_list -> operator_list operator */ 
            temp_tnode_ptr1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Temp_tnode_ptr = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            temp_tnode_ptr2 = Temp_tnode_ptr;
            while (temp_tnode_ptr2->next != NULL)
                temp_tnode_ptr2 = temp_tnode_ptr2->next;
            temp_tnode_ptr2->next = temp_tnode_ptr1;
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case  32  : /* operator -> atom ' */
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = LEFT_MULT;
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;
			
        case  33  : /* operator -> atom ` */
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = RIGHT_MULT;
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case  34  : /* artificial_word -> artificial_word- } */
            break;
			
        case  35  : /* artificial_word- -> artificial_word- : atom */
            temp_tnode_ptr1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            temp_tnode_ptr2 = Temp_tnode_ptr->operand2;
            while (temp_tnode_ptr2->next != NULL)
                temp_tnode_ptr2 = temp_tnode_ptr2->next;
            temp_tnode_ptr2->next = temp_tnode_ptr1;
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        case  36  : /* artificial_word -> W { int : atom */
            Temp_tnode_ptr =   Unexp_tnode_alloc();
            Temp_tnode_ptr->operator = ARTIFICIAL_WORD;
            Temp_tnode_ptr->operand2 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Temp_tnode_ptr->operand1 = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr); 
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            trash = Pop_tnode_ptr(Sem_stack,Sem_sp_ptr);
            Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr);
            break;

        default  :
            printf("Error : Inavlid type\n");
    }
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     Sem_stack -- Semantic stack, stack of pointers to tnodes.   */ 
/* REQUIRES:                                                       */
/*     Sem_sp -- Semantic stack pointer.                           */
/*     Token -- Token number.                                      */
/*     Current_letter -- letter to be stored in the stack.         */
/*     Current_int -- number to be stored in the stack.         */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Get a new tnode. Change its fields to represent a letter or */
/*     an integer.                                                 */
/* NOTE:                                                           */
/*******************************************************************/ 
void Store_semantics(Sem_stack,Sem_sp,Token,Current_letter,Current_int)
struct unexp_tnode *Sem_stack[];
int Sem_sp;
int Token;
char Current_letter;
int Current_int;
{
    struct unexp_tnode *Unexp_tnode_alloc();

    if (Token == LETTER) {
        Sem_stack[Sem_sp] = Unexp_tnode_alloc(); 
        Sem_stack[Sem_sp]->operator = SMALL_LETTER;
        Sem_stack[Sem_sp]->s_letter = Current_letter;
    }
    else if (Token == INT) {
        Sem_stack[Sem_sp] =  Unexp_tnode_alloc();
        Sem_stack[Sem_sp]->operator = SCALAR;
        Sem_stack[Sem_sp]->scalar_num = Current_int;
    }
}


/*******************************************************************/
/* MODIFIES:                                                       */
/*     Sem_stack -- Semantic stack, stack of pointers to tnodes.   */ 
/*     *Sem_sp_ptr -- Semantic stack pointer.                      */
/* REQUIRES:                                                       */
/*     Temp_tnode_ptr -- Pointer to tnode which is to be pushed.   */ 
/* RETURNS: None.                                                  */
/*******************************************************************/ 
void Push_tnode_ptr(Sem_stack,Sem_sp_ptr,Temp_tnode_ptr)
struct unexp_tnode *Sem_stack[];
int *Sem_sp_ptr;
struct unexp_tnode *Temp_tnode_ptr;
{
     if (*Sem_sp_ptr < STACK_SIZE - 1)
         Sem_stack[++(*Sem_sp_ptr)] = Temp_tnode_ptr;
     else
         printf("sematic stack overflow\n");
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     Sem_stack -- Semantic stack, stack of pointers to tnodes.   */ 
/*     *Sem_sp_ptr -- Semantic stack pointer.                      */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to the tnode stored at the top of the stack.        */
/*******************************************************************/ 
struct unexp_tnode *Pop_tnode_ptr(Sem_stack,Sem_sp_ptr)
struct unexp_tnode *Sem_stack[];
int *Sem_sp_ptr;
{
     if ((*Sem_sp_ptr > 0) && (*Sem_sp_ptr < STACK_SIZE))
         return(Sem_stack[(*Sem_sp_ptr)--]);
     else
         printf("sematic stack underflow\n");
}

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Sem_stack -- Semantic stack, stack of pointers to tnodes.   */ 
/*     Sem_sp -- Semantic stack pointer.                           */
/* RETURNS: None.                                                  */
/* NOTE:                                                           */
/*     Called only when DEBUG_PARSE flag is on.                    */
/*******************************************************************/ 
void Print_Sem_stack(Sem_stack,Sem_sp)
struct unexp_tnode *Sem_stack[];
int Sem_sp;
{
    int i;
    for (i=0;i<=Sem_sp;i++) 
        if (Sem_stack[i] != NULL) {
            printf("Sem_stack[%d] = %c\n",i,Sem_stack[i]->s_letter);
            printf("Sem_stack[%d] = %d\n",i,Sem_stack[i]->scalar_num);
        }
}

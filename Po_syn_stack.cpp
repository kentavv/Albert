/*******************************************************************/
/***  FILE :     Po_syn_stack.c                                  ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      void Push_token()                                      ***/
/***      int Get_top_token()                                    ***/
/***      int Get_next_to_top_token()                            ***/
/***      int Pop_token()                                        ***/
/***      void Print_stack()                                     ***/
/***  PRIVARE ROUTINES: None.                                    ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines which manipulate Syntax  ***/
/***      stack These routines are called by the module          ***/
/***      Po_parse_poly.c while doing the parsing.               ***/
/*******************************************************************/

#include <stdio.h>

#include "Po_syn_stack.h"	
#include "Po_parse_poly.h"

#if 0
static void Print_syn_stack(int Stack[], int Sp);
#endif


/*******************************************************************/
/* MODIFIES:                                                       */
/*     Stack -- Syntax stack.                                      */
/*     Sp_ptr -- Syntax stack pointer.                             */
/* REQUIRES:                                                       */
/*     token -- to be pushed onto the stack.                       */
/* RETURNS: None.                                                  */
/*******************************************************************/ 
void Push_token(int Stack[], int *Sp_ptr, int Token)
{
    if (*Sp_ptr < STACK_SIZE - 1)
        Stack[++(*Sp_ptr)] = Token;
    else
        printf(" Stack full\n");
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Stack -- Syntax stack.                                      */
/*     Sp    -- Syntax stack pointer.                             */
/* RETURNS:                                                        */
/*     Token at the top of the stack.                              */
/*******************************************************************/ 
int Get_top_token(int Stack[], int Sp)
{
    return(Stack[Sp]);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Stack -- Syntax stack.                                      */
/*     Sp    -- Syntax stack pointer.                              */
/* RETURNS:                                                        */
/*     Token at second from the top of the Stack.                  */
/*******************************************************************/
int Get_next_to_top_token(int Stack[], int Sp)
{
    return(Stack[Sp-1]);
}


/*******************************************************************/
/* MODIFIES:                                                       */
/*     Stack -- Syntax stack.                                      */
/*     Sp_ptr -- Syntax stack pointer.                             */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Token popped from the top of the Stack.                     */
/*******************************************************************/ 
int Pop_token(int Stack[], int *Sp_ptr)
{
    if ((*Sp_ptr >= 0) && (*Sp_ptr < STACK_SIZE))
        return(Stack[(*Sp_ptr)--]);
    printf(" Stack index out of bounds\n");
    return INVALID_TOKEN;
}


#if 0
/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Stack -- Syntax stack.                                      */
/*     Sp    -- Syntax stack pointer.                              */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Print the tokens stored in the stack.                       */
/*******************************************************************/ 
void Print_syn_stack(int Stack[], int Sp)
{
    int i;
    for (i=0;i<=Sp;i++)
        printf("Stack[%d] = %d\n",i,Stack[i]);
}
#endif

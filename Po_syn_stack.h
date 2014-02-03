#ifndef _PO_SYN_STACK_H_
#define _PO_SYN_STACK_H_

/*******************************************************************/
/***  FILE :     Po_syn_stack.h                                  ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/*******************************************************************/

#define    STACK_SIZE    50

void Push_token(int Stack[], int *Sp_ptr, int Token);
int Get_top_token(int Stack[], int Sp);
int Get_next_to_top_token(int Stack[], int Sp);
int Pop_token(int Stack[], int *Sp_ptr);

#endif

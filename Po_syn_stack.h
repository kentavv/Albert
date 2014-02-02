/*******************************************************************/
/***  FILE :     Po_syn_stack.h                                  ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/*******************************************************************/

#define    STACK_SIZE    50

void push_token();
int get_top_token();
int get_next_to_top_token();
int pop_token();
void print_syn_stack();

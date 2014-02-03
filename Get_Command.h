#ifndef _GET_COMMAND_H_
#define _GET_COMMAND_H_

/*******************************************************************/
/***  FILE :     Get_Command.h                                   ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/*******************************************************************/

#define   MAX_LINE        1000
#define   DB_READ_ALBERT   0 
#define   TRUE   1 
#define   FALSE   0 

typedef struct dalbert_node {
    char *lhs;
    char *rhs;
    struct dalbert_node *next;
} Dalbert_node;

#endif

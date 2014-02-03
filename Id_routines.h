#ifndef _ID_ROUTINES_H_
#define _ID_ROUTINES_H_

/*******************************************************************/
/***  FILE :     Id_routines.h                                   ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/*******************************************************************/

typedef struct id_queue_node {
    char *user_str;
    struct polynomial *identity;
    struct id_queue_node *next;
} id_queue_node;

typedef struct id_queue_head{
    struct id_queue_node *first;
} id_queue_head; 

#endif

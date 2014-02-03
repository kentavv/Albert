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

int Add_id(struct polynomial *Id, char Str[], struct id_queue_head *Id_queue);
int Remove_id(int Id_no, struct id_queue_head *Id_queue);
void Remove_all_ids(struct id_queue_head *Id_queue);
void Print_ids(struct id_queue_head Id_queue);

#endif

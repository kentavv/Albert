#ifndef _ID_ROUTINES_H_
#define _ID_ROUTINES_H_

/*******************************************************************/
/***  FILE :     Id_routines.h                                   ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/*******************************************************************/

#include <list>

#include "Po_parse_exptext.h"

struct id_queue_node {
    char *user_str;
    polynomial *identity;
};

int Add_id(struct polynomial *Id, const char *Str, std::list<id_queue_node> &Id_queue);
bool Remove_id(int Id_no, struct std::list<id_queue_node> &Id_queue);
void Remove_all_ids(struct std::list<id_queue_node> &Id_queue);
void Print_ids(const std::list<id_queue_node> &Id_queue);

#endif

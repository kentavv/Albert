#ifndef _MEMORY_ROUTINES_H_
#define _MEMORY_ROUTINES_H_

#include "Po_prod_bst.h"

struct unexp_tnode *Unexp_tnode_alloc();
void Free_tnode(struct unexp_tnode *Pntr);
PROD_TREEPTR  prod_talloc();
struct polynomial *Poly_alloc();
struct term_node *Term_node_alloc();
struct term_head *Term_head_alloc();
void *Mymalloc(int size);
struct id_queue_node *Id_queue_node_alloc();
void No_memory_panic();
void Short_string_panic();

#endif

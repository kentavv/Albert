#ifndef _PO_EXPAND_POLY_H_
#define _PO_EXPAND_POLY_H_

struct unexp_tnode *Expand_parse_tree(struct unexp_tnode *Unexp_tree);
struct unexp_tnode *Simplify_parse_tree(struct unexp_tnode *Unsimp_tree, int *Modified_ptr);
struct unexp_tnode *Elim_subtraction(struct unexp_tnode *Unsimp_tree, int *Modified_ptr);

#endif

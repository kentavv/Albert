#include <stdio.h>
#include <stdlib.h>

#include "Build_defs.h"
#include "Basis_table.h"
#include "Mult_table.h"

#define assert_not_null(p) if (p == NULL) return(0)

int initGlobals()
{
  char *Mymalloc();
  extern char **Pair_present;
  extern Mt_block ***Mt_block_index;

  int i;

  Basis_table = (BT_rec *)Mymalloc(sizeof(BT_rec) * (DIMENSION_LIMIT + 1));
  assert_not_null(Basis_table);

  PP_COL_SIZE = ((DIMENSION_LIMIT - 1) / 8) + 1;
  Pair_present = (char **)Mymalloc(sizeof(char *) * DIMENSION_LIMIT);
  assert_not_null(Pair_present);
  for(i = 0; i < DIMENSION_LIMIT; ++i){
    Pair_present[i] = ((char *) Mymalloc(sizeof(char) * PP_COL_SIZE));
    assert_not_null(Pair_present[i]);
  }

  MTB_INDEX_SIZE = (DIMENSION_LIMIT/MTB_SIZE) + 1;
  Mt_block_index = (Mt_block ***)Mymalloc(sizeof(Mt_block **) * MTB_INDEX_SIZE);
  assert_not_null(Mt_block_index);
  for(i = 0; i < MTB_INDEX_SIZE; ++i){
    Mt_block_index[i] = (Mt_block **)Mymalloc(sizeof(Mt_block *) * MTB_INDEX_SIZE);
    assert_not_null(Mt_block_index[i]);
  }


  return(1);
}


/* TW 9/27/93 - forgot to free these up */
int freeGlobals()
{
  int i;
  extern char **Pair_present;

  assert_not_null(Basis_table);
  free(Basis_table);

  assert_not_null(Pair_present);
  for(i = 0; i < DIMENSION_LIMIT; ++i){
    assert_not_null(Pair_present[i]);
    free(Pair_present[i]);
  }
  free(Pair_present);
}

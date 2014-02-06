#include <stdio.h>
#include <stdlib.h>

#include "initGlobals.h"
#include "Build_defs.h"
#include "Basis_table.h"
#include "Memory_routines.h"
#include "Mult_table.h"

extern char *Pair_present;
extern Mt_block ***Mt_block_index;

int initGlobals(void)
{
  int i;

  Basis_table = (BT_rec *)Mymalloc(sizeof(BT_rec) * (DIMENSION_LIMIT + 1));
  assert_not_null(Basis_table);

  PP_COL_SIZE = ((DIMENSION_LIMIT - 1) / 8) + 1;
#if 1
  Pair_present = (char *)Mymalloc(sizeof(char) * DIMENSION_LIMIT * PP_COL_SIZE);
  assert_not_null(Pair_present);
#else
  Pair_present = (char **)Mymalloc(sizeof(char *) * DIMENSION_LIMIT);
  assert_not_null(Pair_present);
  for(i = 0; i < DIMENSION_LIMIT; ++i){
    Pair_present[i] = ((char *) Mymalloc(sizeof(char) * PP_COL_SIZE));
    assert_not_null(Pair_present[i]);
  }
#endif

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
void freeGlobals(void)
{
  int i;

  if(Basis_table) {
    free(Basis_table);
    Basis_table = NULL;
  }

  if(Pair_present) {
#if 0
    for(i = 0; i < DIMENSION_LIMIT; ++i){
      free(Pair_present[i]);
      Pair_present[i] = NULL;
    }
#endif
    free(Pair_present);
    Pair_present = NULL;
  }

  if(Mt_block_index) {
    for(i = 0; i < MTB_INDEX_SIZE; ++i){
      if(Mt_block_index[i]) {
        free(Mt_block_index[i]);
        Mt_block_index[i] = NULL;
      }
    }
    free(Mt_block_index);
    Mt_block_index = NULL;
  }
}

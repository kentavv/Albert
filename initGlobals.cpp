#include "initGlobals.h"
#include "Build_defs.h"
#include "Basis_table.h"

int initGlobals(void)
{
  initBasisTable();

  PP_COL_SIZE = ((DIMENSION_LIMIT - 1) / 8) + 1;

  return(1);
}

void freeGlobals(void)
{
  freeBasisTable();
}

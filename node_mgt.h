#ifndef _NODE_MGT_H_
#define _NODE_MGT_H_

/******************************************************************/
/***  FILE :        node_mgt.h                                  ***/
/***  AUTHOR:       David P. Jacobs                             ***/
/***  PROGRAMMER:   David Lee                                   ***/
/***  DATE WRITTEN: May 1990.                                   ***/
/***  MODIFIED:     Aug 1992. David Lee.                        ***/
/***                           Sparse Matrix Code Added.        ***/
/***  PUBLIC ROUTINES:                                          ***/
/***  PRIVATE ROUTINES:                                         ***/
/***  MODULE DESCRIPTION:                                       ***/
/***      The include file for the memory mgt routines in       ***/
/***      node_mgt.c. The #define RECORDS_PER_BLOCK is the      ***/
/***      number of node records per allocated block of memory. ***/
/***      8190 is chosen because it is near a 64 K boundary     ***/
/***      and this should make the system call to malloc        ***/
/***      perform better.                                       ***/
/******************************************************************/

#define RECORDS_PER_BLOCK   8190

typedef struct block {
                            Node  record[RECORDS_PER_BLOCK];
                            struct block *next_block;
#ifdef DEBUGMM
                            short int id;
#endif
} mem_block;

#endif

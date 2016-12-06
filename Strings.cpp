/*******************************************************************/
/***  FILE :     Strings.c                                       ***/
/***  AUTHOR:    Jeff Offutt                                     ***/
/***  PROGRAMMER:Sekhar Muddana                                  ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      void Str_cat()                                         ***/
/***  PRIVATE ROUTINES: None.                                    ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines to do operations on      ***/
/***      strings whose size is not fixed.                       ***/ 
/*******************************************************************/

#include <stdlib.h>
#include <string.h>

#include "Strings.h"
#include "Memory_routines.h"

/*******************************************************************/
/* MODIFIES:                                                       */
/*     Str1_ptr -- pointer to a string                             */
/*     *Str1_maxsize_ptr -- maximum size to which Str1 can grow.   */ 
/* REQUIRES:                                                       */
/*     Str2 -- string which is to be attached to *Str1_ptr.        */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Str2 is concatenated to Str1.                               */
/* NOTE:                                                           */
/*     If Str1 is not big enough to be concatenated with str2,     */
/*     extra space is allocated, *str1_ptr is changed to point to  */
/*     Str1 whose size has expanded and *Str1_maxsize_ptr is       */
/*     changed to reflect the increased size of Str1.              */
/*     If unable to find space, No_memory_panic() will be called.  */
/*******************************************************************/ 
void Str_cat(char **Str1_ptr, char Str2[], int *Str1_maxsize_ptr)
{
    if (*Str1_maxsize_ptr < (int)strlen(*Str1_ptr) + (int)strlen(Str2) + 1) {
        int n = 2 * (*Str1_maxsize_ptr + strlen(Str2));

        *Str1_ptr = (char*) realloc(*Str1_ptr, n);

        if (*Str1_ptr == NULL)
            No_memory_panic();
        else {
            strcat(*Str1_ptr, Str2);
            *Str1_maxsize_ptr = n;
        }
    }
    else
        strcat(*Str1_ptr, Str2);
}

#include <stdio.h>
#include <unistd.h>

#include "getchar.h"

/*
 * Called from two places. From my_getline() in this module and from
 * Po_routines.c. Its purpose is to fix the bug of cntl D which takes
 * Albert into infinite loop. Basically we replace standard bufferd
 * getchar() with unbuffered getchar(). Code is copied from KR C book.
 * Looks like the bug is hardware dependent.
 */

int getchar(void)
{
   char c;
   fflush(stdout);
   return read(0, &c, 1) == 1 ? (unsigned char) c : EOF;
}

/*
 * Static function. reads a line from the terminal and returns line
 * length. Code copied from KR C book.
 */

int my_getline(char s[], int lim)
{
    int i;

    for (i=0; i < lim-1; i++) {
        int c = getchar(); 
        if(c == EOF || c == '\n' || c == '\r') {
          break;
        }
        s[i] = c;
    }

    s[i] = '\0';

    return i ;
}


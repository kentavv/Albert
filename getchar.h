#ifndef _GETCHAR_H_
#define _GETCHAR_H_

#undef getchar           /* Use unbuffered getchar(). */

int getchar(void);
int my_getline(char s[], int lim);

#endif

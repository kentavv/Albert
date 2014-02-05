#ifndef _HELP_H_
#define _HELP_H_

/*******************************************************************/
/***  FILE :     Help.h                                          ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  MODIFIED:  David Lee (8/20/92) - Added help for change     ***/
/***                                   command.                  ***/
/***             Trent Whiteley (8/20/93) - rewrote the help     ***/
/***                                   messages                  ***/
/*******************************************************************/

void initHelp(void);
int Help(char topic[]);
void more(int *lines);

#endif

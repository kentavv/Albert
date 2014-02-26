/**********************************************************************/
/***  FILE :     Help.c                                             ***/
/***  AUTHOR:    Trent Whiteley                                     ***/
/***  MODIFIED:                                                     ***/
/***                                                                ***/
/***  PUBLIC ROUTINES:                                              ***/
/***    int Help();                                                 ***/
/***  PRIVATE ROUTINES:                                             ***/
/***    char * getHelp();                                           ***/
/***    void displayHelp();                                         ***/
/***  MODULE DESCRIPTION:                                           ***/
/***    This module contains routines used to handle the help       ***/
/***    system for albert.                                          ***/
/**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Help.h"
#include "Help_pri.h"
#include "Get_Command.h"

static const char *getHelp(const char *helpRqst);
static void displayHelp(const char *helpPtr);

void initHelp(void)
{
  helpLines = 24; /* resonable defaults */
  helpCols = 80;
}

int Help(char */*topic[]*/)
{
  char *str = NULL;
  const char *helpPtr = NULL; /*, *test;*/

  displayHelp(getHelp("H"));

  str = rl_gets("HELP-->");
  while(strlen(str)){
    if(str && strlen(str) > 0) {
      helpPtr = getHelp(str);
      if(helpPtr) {
        displayHelp(helpPtr);
      } else {
        printf("No help is available for %s\n", str);
      }
    }
    printf(" Type a letter for help, or carriage return to\n return\
	to the Albert session.\n\n\n");
    str = rl_gets("HELP-->");
  }
  return 1;
}




const char *getHelp(const char *helpRqst)
{
  int i;

  for(i = 0; i < NUM_COMMANDS; ++i){
    if(!strncmp(helpRqst, Help_lines[i].help_topic, strlen(helpRqst))){
      return(Help_lines[i].help_text);
    }
  }
  return(NULL);
}




void displayHelp(const char *helpPtr)
{
  puts(helpPtr);
}


void more(int *lines)
{
  return;

  if(*lines >= helpLines-2){
    *lines = 0;
    printf("\nHit Return to continue-->");    /* print more message at bottom left of screen */
    fflush(stdout);
    getchar();
    printf("\n");
  }
}

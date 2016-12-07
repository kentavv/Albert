/*********************************************************************/
/***  FILE :     Get_Command.c                                     ***/
/***  AUTHOR:    Sekhar Muddana                                    ***/
/***  MODIFICATION: 9/93 - Trent Whiteley                          ***/
/***                       added code to skip past leading white   ***/
/***                       space on the Albert command line and    ***/
/***                       allow for the specification of an       ***/
/***                       alternate directory for .albert file    ***/
/***  PUBLIC ROUTINES:                                             ***/
/***      int ReadDotAlbert()                                      ***/
/***      int GetCommand()                                         ***/
/***      int Substr()                                             ***/
/***      int getchar()                                            ***/
/***  PRIVATE ROUTINES:                                            ***/
/***      int DoSubstitution()                                     ***/
/***      int my_getline()                                         ***/
/***  MODULE DESCRIPTION:                                          ***/
/***      Accept command line from the user.                       ***/
/***      Make use of definitions given in .albert if necessary.   ***/
/***      Return the Command and the Operand to the driver().      ***/
/*********************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "Get_Command.h"
#include "Memory_routines.h"
#include "Strings.h"
#include "Type_table.h"

static int DoSubstitution(void);

static char *total_line = NULL;
static int total_len = 0;
static char *total_line1 = NULL;
static Dalbert_node *Dalbertheadptr = NULL;
static int DotalbertPresent = 0;


/* Based on http://cnswww.cns.cwru.edu/php/chet/readline/readline.html */

/* A static variable for holding the line. */
static char *line_read = (char *)NULL;

/* Read a string, and return a pointer to it.
 *    Returns NULL on EOF. */
char *rl_gets(const char *prompt)
{
  /* If the buffer has already been allocated,
 *      return the memory to the free pool. */
  if (line_read) {
      free (line_read);
      line_read = (char *)NULL;
    }

  do {
  /* Get a line from the user. */
  line_read = readline (prompt);
  } while(!line_read);

  /* If the line has any text in it,
 *      save it on the history. */
  if (line_read && *line_read) {
    add_history (line_read);
  }

  return (line_read);
}


/* Called from Main(). Returns the command and the operand.
 * Also uses the definitions in .albert. Since both comand and
 * operand are strings whose length is not fixed (a long command
 * result in long string length), we keep track of string lengths
 * and do dynamic string expansion by calling Str_cat().
 */

void GetCommand(char **Command_ptr, char **Operand_ptr, int *Command_len_ptr, int *Operand_len_ptr)
{
    char temp_line2[MAX_LINE];
    int slashfound;
    int temp_line_len;
    int i,j;
    int tl;

    total_line[0] = '\0';
    char *temp_line = rl_gets("-->");
  if(temp_line) {
    temp_line_len = strlen(temp_line);
    slashfound = TRUE;
    while ((slashfound) && (temp_line_len > 0)) {
        i = 0;
        j = 0;
	while(temp_line[i] == ' '){	/* TW - read past leading white space */
	  ++i;
	}				/* TW - end */
        while ((temp_line[i] != '\\') && (temp_line[i] != '\0'))
            temp_line2[j++] = temp_line[i++];
        if (temp_line[i] == '\\') {
            slashfound = TRUE;
            temp_line = rl_gets("...");
            temp_line_len = strlen(temp_line);
        }
        else
            slashfound = FALSE;
        temp_line2[j] = '\0';
        Str_cat(&total_line,temp_line2,&total_len);
    }
    i = 0;
    tl = strlen(total_line);
    total_line1 = (char*) Mymalloc(tl+1);
    while (i < tl) {
        if ((total_line[i] == ' ') || (total_line[i] == '\t'))
            break;
        total_line1[i] = total_line[i];
        i++;
    }
    total_line1[i++] = '\0';
    (*Command_ptr)[0] = '\0';
    Str_cat(Command_ptr,total_line1,Command_len_ptr);
    j = 0;
    while (i < tl) {
        if ((total_line[i] != ' ') && (total_line[i] != '\t'))
            total_line1[j++] = total_line[i++];
        else
            i++;
    }
    total_line1[j] = '\0';
    strcpy(total_line,total_line1);

    if (DotalbertPresent) {
        while (DoSubstitution());
    }

    (*Operand_ptr)[0] = '\0';
    Str_cat(Operand_ptr,total_line,Operand_len_ptr);
#if DB_READ_ALBERT
    printf("command = %s \n",*Command_ptr);
    printf("operand = %s \n",*Operand_ptr);
#endif
    free(total_line1);
  }
  if (line_read) {
      free (line_read);
      line_read = (char *)NULL;
    }
}

/*
 * Static function. It scans the total_line and breaks it into 3 strings
 * str1, str2 and str3. str1 is the prefix before $ in total_line.
 * str2 is the string between 2 $'s. str3 is the tail. Now str2 is
 * substituted with a string as per the definitions in .albert. i.e if
 * str2 matches with some left hand side of a definition in .albert,
 * then it is replaced with corresponding right hand side.
 */

int DoSubstitution(void)
{
    char *str1;
    char *str2;
    char *str3;
    int tl;
    int i,j;
    Dalbert_node *tnodeptr;
    int match = 0;

    tnodeptr = Dalbertheadptr;
    tl = strlen(total_line);
    if (tl < 3)
        return(0);

    str1 = (char*) Mymalloc(tl+1);
    i = 0;
    j = 0;
    while ((i<tl) && (total_line[i] != '$'))
        str1[j++] = total_line[i++];
    str1[j] = '\0';
    if (total_line[i++] != '$') {
        free(str1);
        return(0);
    }
    str2 = (char*) Mymalloc(tl+1);
    j = 0;
    while ((i<tl) && (total_line[i] != '$'))
        str2[j++] = total_line[i++];
    str2[j] = '\0';
    if (total_line[i++] != '$') {
        free(str1);
        free(str2);
        return(0);
    }
    str3 = (char*) Mymalloc(tl+1);
    j = 0;
    while (i<tl)
        str3[j++] = total_line[i++];
   str3[j] = '\0';

    while ((tnodeptr->next != NULL) && (!match)) {
        if (strcmp(tnodeptr->lhs,str2) == 0) {
            match = 1;
            break;
        }
        tnodeptr = tnodeptr->next;
    }
    if (!match) {
        free(str1);
        free(str2);
        free(str3);
        return(0);
    }
    total_line[0] = '\0';
    Str_cat(&total_line,str1,&total_len);
    Str_cat(&total_line,tnodeptr->rhs,&total_len);
    Str_cat(&total_line,str3,&total_len);
#if DB_READ_ALBERT
    printf("str1 = %s \n",str1);
    printf("str2 = %s \n",str2);
    printf("str3 = %s \n",str3);
    printf("total line = %s \n",total_line);
#endif
    free(str1);
    free(str2);
    free(str3);
    return(1);
}

/* Called from the Main() as a part of initialization.
 * reads the .albert file if present and forms a list of definitions.
 * Linked list representation means there could be any number of
 * definitions in .albert.
 * A definition can start anywhere on a line.
 * Blank lines are permitted anywhere in .albert.
 * Line after % is totally ignored.
 * A definition can be continued on the next line by ending it with a /.
 * Comments can appear between two lines of a definition or at the of
 * any line.
 * A blank ' ' or a '\t' seperates the left hand side and right hand
 * side of a definition.
 * If Albert dumps the core during initialization, probably there is a
 * bug in this routine. Albert can be used either by changing the .albert
 * file or by getting rid of it completely.
 */

int ReadDotAlbert(Dalbert_node *dalbert_node_ptr, char *albertFileLoc) /* TW - location of the .albert file */
{
    FILE *albert_fp;
    char temp_line[MAX_LINE];
    char temp_line2[MAX_LINE];
    char cwd[200];
    int temp_line_len;
    int i = 0,j = 0;
    int slashfound = TRUE;
    char *temp;
    int lhs_len,rhs_len;
    int alinelen;
    Dalbert_node *tnodeptr;
    int lhsstart;
    /*int rhsstart;*/
    int blankfound = TRUE;

    char *albert_line;
    int albert_line_len;

    int inCWD;		/* TW 10/8/93 - flag to determine if file is in cwd */


    total_len = MAX_LINE;
    total_line = (char*) Mymalloc(total_len);

    Dalbertheadptr = dalbert_node_ptr;
    tnodeptr = dalbert_node_ptr;

    if(!strncmp(".albert", albertFileLoc, strlen(albertFileLoc))){
      getcwd(cwd, 200);
      strcpy(albertFileLoc, cwd);
      strcat(albertFileLoc, "/.albert");
      inCWD = TRUE;
    }
    else{
      getcwd(cwd, 200);
      inCWD = FALSE;
    }

    if((albert_fp = fopen(albertFileLoc,"r")) == NULL){	/* TW - check the
specified path 1st */
      if(!inCWD){
        printf("Can't open %s file.\n", albertFileLoc);	/* TW - changed
message to display path */
        printf("Attempting to read %s/.albert\n", cwd);	/* TW - added message */
        if((albert_fp = fopen(".albert", "r")) == NULL){
          printf("Can't open %s/.albert file.\n", cwd);	/* TW - added cwd */
          DotalbertPresent = FALSE;
          return(0);
        }
	else{
	  printf("Using %s/.albert\n", cwd);
	}
      }
      else{
        printf("Can't open %s/.albert file.\n", cwd);	/* TW - added cwd */
        DotalbertPresent = FALSE;
        return(0);
      }
    }
    else{
      printf("Using %s\n", albertFileLoc);
    }

    DotalbertPresent = TRUE;

    albert_line_len = MAX_LINE;
    albert_line = (char*) Mymalloc(albert_line_len);
    albert_line[0] = '\0';
    while ((temp = fgets(temp_line,MAX_LINE,albert_fp)) != NULL) {
        slashfound = TRUE;
        while ((slashfound) && (temp != NULL)) {
            blankfound = TRUE;
            while ((blankfound) && (temp != NULL)) {
                temp_line_len = strlen(temp_line);
                for (i=0;i<temp_line_len;i++) {
                    if (temp_line[i] == '%') {
                        temp_line[i] = '\0';
                        break;
                    }
                }
                temp_line_len = strlen(temp_line);
                for (i=0;i<temp_line_len;i++) {
                    if ((temp_line[i] != ' ') && (temp_line[i] != '\t') &&
                        (temp_line[i] != '\n'))
                        blankfound = FALSE;
                }
                if (blankfound)
                    temp = fgets(temp_line,MAX_LINE,albert_fp);
            }
            i = 0;
            j = 0;
            while ((temp_line[i] != '\\') && (temp_line[i] != '\0') &&
                   (temp_line[i] != '%') && (temp_line[i] != '\n'))
                temp_line2[j++] = temp_line[i++];
            if (temp_line[i] == '\\') {
                slashfound = TRUE;
                temp = fgets(temp_line,MAX_LINE,albert_fp);
            }
            else
                slashfound = FALSE;
            temp_line2[j] = '\0';
            Str_cat(&albert_line,temp_line2,&albert_line_len);
        }

        i = 0;
        alinelen = strlen(albert_line);
        while (i < alinelen) {
            if ((albert_line[i] != ' ') && (albert_line[i] != '\t'))
                break;
            i++;
        }
        lhs_len = 0;
        lhsstart = i;
        while (i < alinelen) {
            if ((albert_line[i] == ' ') || (albert_line[i] == '\t'))
                break;
            lhs_len++;
            i++;
        }
        rhs_len = 0;
        while (i < alinelen) {
            if ((albert_line[i] != ' ') && (albert_line[i] != '\t'))
                rhs_len++;
            i++;
        }
        if ((lhs_len > 0) && (rhs_len > 0))  {
            tnodeptr->lhs = (char*) Mymalloc(lhs_len+1);
            i = lhsstart;
            j = 0;
            while (i < alinelen) {
                if ((albert_line[i] == ' ') || (albert_line[i] == '\t'))
                    break;
                tnodeptr->lhs[j++] = albert_line[i++];
                tnodeptr->lhs[j] = '\0';
            }

            tnodeptr->rhs = (char*) Mymalloc(rhs_len+1);
            j = 0;
            while (i < alinelen) {
                if ((albert_line[i] != ' ') && (albert_line[i] != '\t')) {
                     tnodeptr->rhs[j++] = albert_line[i];
                     tnodeptr->rhs[j] = '\0';
                }
                i++;
            }

            tnodeptr->next = (Dalbert_node *) Mymalloc(sizeof(Dalbert_node));
            tnodeptr = tnodeptr->next;
            tnodeptr->lhs = NULL;
            tnodeptr->rhs = NULL;
            tnodeptr->next = NULL;
        }
        albert_line[0] = '\0';
    }

#if DB_READ_ALBERT
        i = 1;
        tnodeptr = dalbert_node_ptr;
        while (tnodeptr->next != NULL) {
            printf(" %d  %d %s  %d %s \n",i++,strlen(tnodeptr->lhs),
                     tnodeptr->lhs,strlen(tnodeptr->rhs),tnodeptr->rhs);
            tnodeptr = tnodeptr->next;
        }
#endif
     free(albert_line);
     return(1);
}

/*
 * Called from driver.c and Help.c. Used to check if Str1 is a string
 * which is a prefix of Str2.
 */

int Substr(const char Str1[], const char Str2[])
{
  return strncmp(Str1, Str2, strlen(Str1)) == 0;
}



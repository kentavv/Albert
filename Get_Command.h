#ifndef _GET_COMMAND_H_
#define _GET_COMMAND_H_

/*******************************************************************/
/***  FILE :     Get_Command.h                                   ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/*******************************************************************/

#define   MAX_LINE        1000
#define   DB_READ_ALBERT   0 
#define   TRUE   1 
#define   FALSE   0 

typedef struct dalbert_node {
    char *lhs;
    char *rhs;
    struct dalbert_node *next;
} Dalbert_node;

void GetCommand(char **Command_ptr, char **Operand_ptr, int *Command_len_ptr, int *Operand_len_ptr);
int ReadDotAlbert(Dalbert_node *dalbert_node_ptr, char *albertFileLoc);
int Substr(const char Str1[], const char Str2[]);
char *rl_gets(const char *prompt);

#endif

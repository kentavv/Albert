/***********************************************************************/
/***  FILE :     driver.c                                            ***/
/***  AUTHOR:    Sekhar Muddana                                      ***/
/***  MODIFIED:  David Lee   August 1992.                            ***/
/***                   - Added the change command                    ***/
/***                   - Added information to the display command    ***/
/***                   - Altered version information and title       ***/
/***             9/93 - Trent Whitley                                ***/
/***		        Removed load, store, and catalog cmds and    ***/
/***                    implemented view, save, & output commands    ***/
/***                    also implemented command line arguements, -d ***/
/***                    and -a, and initialize and free global vars  ***/
/***             10/93 - Trent Whiteley                              ***/
/***                     changes made to implement interrupt handler ***/
/***  PUBLIC ROUTINES:                                               ***/
/***      Scalar GetField()                                          ***/
/***  PRIVATE ROUTINES:                                              ***/
/***      void Print_title()                                         ***/
/***      Type CreateTargetType()                                    ***/
/***      int Compatible()                                           ***/
/***  MODULE DESCRIPTION:                                            ***/
/***    This is the Main module of Albert.                           ***/
/***    It does initializations.                                     ***/
/***    Interprets and executes Commands.                            ***/
/***    Gets Command line and executes the command.                  ***/
/***********************************************************************/

#include <list>

using std::list;

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <omp.h>

#include "driver.h"
#include "Basis_table.h"
#include "Build.h"
#include "Build_defs.h"
#include "Field.h"
#include "Generators.h"
#include "Get_Command.h"
#include "Help.h"
#include "Id_routines.h"
#include "Memory_routines.h"
#include "Po_create_poly.h"
#include "Po_parse_exptext.h"
#include "Po_routines.h"
#include "Scalar_arithmetic.h"
#include "Ty_routines.h"
#include "Type_table.h"
#include "Mult_table.h"

static void Print_title();
static Type CreateTargetType(struct P_type ptype);
static int Compatible(struct polynomial *Poly, struct P_type ptype);
static void usage();
static void sigCatch(int x);

static const char *enhanced_version = "4.0.13";
static const unsigned short mult_file_version = 0xabef;
static bool save_tables(const char *filename);
static bool restore_tables(const char *filename);


#define  NOT_PRESENT  0
#define  PRESENT     1
#define  COMMAND_LEN  15


Scalar Field = DEFAULT_FIELD;          /* Build_defs.h */

int sigIntFlag = 0;		/* TW 10/8/93 - flag for Ctrl-C */
jmp_buf env;

int main(int argc, char *argv[])
{
/* TW 9/18/93 - code to support save, view, & output commands */
    char tableFileName[100];
    char table;
    FILE *tableFilePtr = nullptr;

    Type Target_type = nullptr;

/* TW 9/18/93 - end of code to support save, view, & output commands */

    char *Command;
    char *Operand;
    int Command_len;
    int Operand_len;
    Dalbert_node Dalberthead; /* Head to the list of .albert defs. */

    list<id_queue_node> Id_queue; /* head to Queue of identities */
    int Cur_id_no;
    struct polynomial *Cur_id;
    struct polynomial *Cur_poly;
    int Cur_poly_len;     /* length of the expanded Cur_poly string */
    P_type ptype,Temp_type;   /* structure for problem type */
    char c;

    int mtable_status = NOT_PRESENT;
    int problem_type_present = FALSE;
    int quit = 0;

    int out_of_memory = FALSE;

    int i; /*,j;*/
    int first_generator;

    /* TW 9/8/93 - BEGIN argument handling */
    char dir[100];
    int argPos;
    /*FILE *fp;*/

    dir[0] = '\0';			/* initialize dir */
    if(argc > 5){
      usage();
      exit(1);
    }
    else{
      for(i = 1; i < argc; ++i){
        if(argv[i][0] != '-'){            /* make sure it's a flag */
            printf("flag, %s, requires a \"-\".\n", argv[i]);
            usage();
        }
        else{
	  if(strlen(argv[i]) > 2){
	    argPos = 2;
	  }
	  else{
	    argPos = 0;
	  }
          switch(argv[i][1]){             /* get the flag parameters */
            case 'a':                     /* .albert directory */
	      if(argPos){
		strcpy(dir, argv[i] + argPos);
	      }
	      else{
	        ++i;
                strcpy(dir, argv[i]);
	      }
              if(dir[strlen(dir)-1] != '/'){
                strcat(dir, "/");
              }
              strcat(dir, ".albert");
/*              if(!(fp = fopen(dir, "r"))){
                printf("%s does not exist.\n", dir);
              }*//* TW - check this in Get_Command() */
              break;
            default:
              printf("%c is an invalid flag type.\n", argv[i][1]);
              usage();
          }
        }
      }
      if(!strlen(dir)){
	strcpy(dir, ".albert");
      }
    }

    initHelp();
    /* TW 9/8/93 - END argument handling */

    Print_title();

    Dalberthead.lhs = nullptr;
    Dalberthead.rhs = nullptr;
    Dalberthead.next = nullptr;
    ReadDotAlbert(&Dalberthead, dir);
    printf("Type h for help.\n");
    Command_len = COMMAND_LEN;
    Operand_len = MAX_LINE;
    Command =(char*) Mymalloc(Command_len);
    Operand =(char*) Mymalloc(Operand_len);

    S_init();

    /* initialize the problem type */
    for (i = 0;i<NUM_LETTERS;i++) {
        ptype.degrees[i] = 0;
    }
    ptype.tot_degree = 0;

    /* TW 10/5/93 - Ctrl-C sig handler instantiation */
    {
      struct sigaction na;
     
      na.sa_handler = sigCatch;
      sigemptyset(&na.sa_mask);
      na.sa_flags = 0;
     
      sigaction(SIGINT, &na, nullptr);
    }

    while (!quit) {

        if (setjmp(env) != 0) {
/* Coming here after a long jump. Memory overflow. */
            if (mtable_status == PRESENT) {
                DestroyTypeTable();
                DestroyMultTable();
                mtable_status = NOT_PRESENT;
                printf("Tables destroyed.\n");
            }
            printf("Command unsuccessfull\n");
        }

	if(sigIntFlag){		/* TW 10/5/93 - Ctrl-C check */
	  printf("We have detected a Ctrl-C.\n");
	  printf("Type <quit> to quit.\n");
	  sigIntFlag = 0;
	}

        printf("\n\n");
        //printf("\n-->");
        GetCommand(&Command,&Operand,&Command_len,&Operand_len);
        printf("\n");

        switch (Command[0]) {

            case 'i':

/* "identity polynomial". Accept new identity. */

                 if (!Substr(Command,"identity")) {
                     printf("Illegal command.");
                     break;
                 }
                 out_of_memory = FALSE;
                 Cur_id = Create_poly(Operand,&Cur_poly_len,&out_of_memory);
                 if (Cur_id != nullptr) {
                     if (!Homogeneous(Cur_id)) {
                         printf("Given identity is not homogeneous.");
                         break;
                     }
                     else if (Cur_id->degree < 2) {
                         printf("Total degree must be at least 2.");
                         break;
                     }
                     else {
                         AssignNumbersToLetters(Cur_id);
                         Print_poly(Cur_id,Cur_poly_len);
                         printf("\n");
                         Cur_id_no = Add_id(Cur_id,Operand, Id_queue);
                         printf("Entered as identity %d.\n",Cur_id_no);
                         if (mtable_status == PRESENT) {
                             DestroyMultTable();
                             mtable_status = NOT_PRESENT;
                             printf("Destroyed the Multiplication Table.\n");
                         }
                     }
                 }
                 else {
                     if (!out_of_memory)
                         printf("%s is not a polynomial.",Operand);
                 }
                 break;

            case 'r':

/* "remove number|*". remove identity. */

                 if (!Substr(Command,"remove")) {
                     printf("Illegal command.");
                     break;
                 }
                 if (Operand[0] == '*') {
                     Remove_all_ids(Id_queue);
                     printf("Removed all the identities.\n");
                     if (mtable_status == PRESENT) {
                         DestroyMultTable();
                         mtable_status = NOT_PRESENT;
                         printf("Destroyed the Multiplication Table.\n");
                     }
                 }
                 else {
                     Cur_id_no = atoi(Operand);
                     if (Cur_id_no > 0) {
                         if (Remove_id(Cur_id_no, Id_queue)) {
                             printf("Identity %d removed.\n",Cur_id_no);
                             if (mtable_status == PRESENT) {
                                 DestroyMultTable();
                                 mtable_status = NOT_PRESENT;
                                 printf("Destroyed the Multiplication Table.\n");
                             }
                         }
                         else
                             printf("Identity %d not present.",Cur_id_no);
                        }
                     else
                         printf("Number must be at least 1.");
                 }
                 break;

            case 'g':

/* "generators word". Accept problem type. */

                 if (!Substr(Command,"generators")) {
                     printf("Illegal command.");
                     break;
                 }
                 if (!Parse_generator_word(Operand,&Temp_type))
                     printf("Error in the generator list.");
                 else if (Temp_type.tot_degree < 2)
                     printf("Total degree must be at least 2.");
                 else {
                     for (i = 0;i<NUM_LETTERS;i++)
                         ptype.degrees[i] = 0;
                     ptype.tot_degree = 0;

                     for (i = 0;i<NUM_LETTERS;i++)
                         ptype.degrees[i] = Temp_type.degrees[i];
                     ptype.tot_degree = Temp_type.tot_degree;
                     if (problem_type_present) {
                         free(Target_type);
                         printf("Problem type changed.\n");
                     }
                     else
                         printf("Problem type stored.\n");
                     AssignBasisNumberstoLetters(ptype);
                     Target_type = CreateTargetType(ptype);
                     problem_type_present = TRUE;
                     if (mtable_status == PRESENT) {
                         DestroyMultTable();
                         mtable_status = NOT_PRESENT;
                         printf("Destroyed the Multiplication and Basis Tables.\n");
                     }
                 }
                 break;

            case 'f':

/* "field number". Change the filed.  */

                 if (!Substr(Command,"field")) {
                     printf("Illegal command.");
                     break;
                 }
                 i = atoi(Operand);
                 if (Change_field(i,&Field)) {
                     printf("Field changed to %d.\n",Field);
                     S_init();
                     if (mtable_status == PRESENT) {
                         DestroyMultTable();
                         mtable_status = NOT_PRESENT;
                         printf("Destroyed the Multiplication Table.\n");
                     }
                 }
                 else
                     printf("Field not changed.");
                 break;

            case 'b':

/* "build". Build the multiplication table. */

                 if (!Substr(Command,"build")) {
                     printf("Illegal command.");
                     break;
                 }
                 if (!problem_type_present) {
                     printf("Problem Type not present.\n");
                     printf("Can't build Multiplication Table.\n");
                     break;
                 }
                 if (mtable_status == PRESENT) {
                     DestroyMultTable();
                     mtable_status = NOT_PRESENT;
                     printf("Destroyed the Multiplication Table.\n");
                 }
                 printf("Building the Multiplication Table.\n");

                 if (setjmp(env) != 0) {
/* Memory overflow while building the multiplication table. */
                     DestroyTypeTable();
                     DestroyMultTable();
                     mtable_status = NOT_PRESENT;
                     printf("Tables destroyed.\n");
                     break;
                 }

                 if (Build(Id_queue, Target_type) == OK)
                     mtable_status = PRESENT;
                 else
                     mtable_status = NOT_PRESENT;
		 if(sigIntFlag){	/* TW 10/5/93 - Ctrl-C check */
		   sigIntFlag = 0;
		   DestroyMultTable();
		   DestroyTypeTable();
		   printf("Build interrupted by Ctrl-C.\n Multiplication table destroyed.\n");
		 }
                 break;

            case 'd':

/* "display". Display the current problem configuration. */

                 if (!Substr(Command,"display")) {
                     printf("Illegal command.");
                     break;
                 }
                 if (!Id_queue.empty()) {
                     printf("Defining identities are:\n");
                     Print_ids(Id_queue);
                 }
                 else
                     printf("No defining identities.\n");
                 printf("Field = %d.\n",Field);

/* Print the Problem type in a good looking format!  */

                 if (problem_type_present) {
                     first_generator = TRUE;
                     printf("Problem type = [");
                     for (i=0;i<NUM_LETTERS;i++) {
                         if (ptype.degrees[i] == 1) {
                             c = 'a' + i;
                             if (!first_generator)
                                 printf(",%c",c);
                             else {
                                 first_generator = FALSE;
                                 printf("%c",c);
                             }
                         }
                         else if (ptype.degrees[i] > 1) {
                             c = 'a' + i;
                             if (!first_generator)
                                 printf(",%d%c",ptype.degrees[i],c);
                             else {
                                 first_generator = FALSE;
                                 printf("%d%c",ptype.degrees[i],c);
                             }
                         }
                     }
                     printf("]; Total degree = %d.\n",ptype.tot_degree);

                 }
                 else
                     printf("Problem type not specified.\n");

                 if (mtable_status == PRESENT)
                     printf("Multiplication table present.");
                 else
                     printf("Multiplication table not present.");
                 break;

            case 'p':

/* "polynomial poly". Enquire if poly is an identity. */

                 if (!Substr(Command,"polynomial")) {
                     printf("Illegal command.");
                     break;
                 }
                 if (mtable_status == NOT_PRESENT) {
                     printf("Multiplication table not present.");
                     break;
                 }
                 out_of_memory = FALSE;
                 Cur_poly = Create_poly(Operand,&Cur_poly_len,&out_of_memory);
                 if (Cur_poly == nullptr) {
                     if (!out_of_memory)
                         printf("%s is not a polynomial.",Operand);
                     break;
                 }
                 if (!Homogeneous(Cur_poly)) {
                     printf("Given polynomial is not homogeneous.");
                     break;
                 }
                 if (Cur_poly->degree < 2) {
                     printf("Total degree must be at least 2.");
                     break;
                 }
                 if (!Compatible(Cur_poly,ptype)) {
                     printf("Polynomial type not a subtype of Target_type.");
                     break;
                 }
/*
                 Print_poly(Cur_poly,Cur_poly_len);
                 printf("\n");
*/
                 if (IsIdentity(Cur_poly))
                     printf("Polynomial is an identity.");
                 else
                     printf("Polynomial is not an identity.");
                 DestroyPoly(Cur_poly);
                 break;

            case 'x':

/* "xpand poly". */

                 if (!Substr(Command,"xpand")) {
                     printf("Illegal command.");
                     break;
                 }
                 out_of_memory = FALSE;
                 Cur_poly = Create_poly(Operand,&Cur_poly_len,&out_of_memory);
                 if (Cur_poly != nullptr) {
                     if (!Homogeneous(Cur_poly)) {
                         printf("Given polynomial is not homogeneous.");
                         DestroyPoly(Cur_poly);
                         break;
                     }
                     else {
                         Print_poly(Cur_poly,Cur_poly_len);
                         printf("\n");
                         DestroyPoly(Cur_poly);
                     }
                 }
                 else {
                     if (!out_of_memory)
                         printf("%s is not a polynomial.",Operand);
                 }
                 break;

/* "save" the multiplication table or the basis table to a file */
            case 's':
                if(!Substr(Command, "save")){
                    printf("Illegal command.");
                    break;
                }

                table = Operand[0];                    /* get the table
type to show */
                if (table != 'b' && table != 'm') {
                    printf("Invalid table type.  Specify \"m\" or \"b\".\n");
                    break;
                } else if(mtable_status != PRESENT) {
                    if (table == 'b') {
                        printf("Basis Table not present.\n");
                    } else {
                        printf("Multiplication Table not present.\n");
                    }
                } else {
                    const char *fn = rl_gets("File Name --> ");
                    printf("\n");
                    if(fn && *fn){
                        FILE *f = fopen(fn, "w");
                        if(!f){
                            printf("Unable to open file, %s.\n", fn);
                            break;
                        }
                        if (table == 'b') {
                            PrintBasisTable(f);
                        } else {
                            Print_MultTable(f);
                        }
                        fclose(f);
                    }
                    else{
                        printf("No file name was entered.  Command aborted.\n");
                        break;
                    }
                }
                break;

/* "checkout" save or restore the multiplication table from a file */
            case 'c':
                if(!Substr(Command, "checkpoint")){
                    printf("Illegal command.");
                    break;
                }

                table = Operand[0];
                if (table != 'r' && table != 's') {
                    printf("Invalid command.  Specify \"l\" or \"s\".\n");
                    break;
                } else if(mtable_status != PRESENT && table == 's') {
                    printf("Multiplication Table not present.\n");
                } else {
                    const char *fn = rl_gets("File Name --> ");
                    printf("\n");
                    if(fn && *fn){
                        if (table == 'r') {
                            if(!restore_tables(fn)) {
                                printf("Unable to load tables from %s\n", fn);
                            } else {
                                mtable_status = PRESENT;
                            }
                        } else {
                            if(!save_tables(fn)) {
                                printf("Unable to save tables to %s\n", fn);
                            }
                        }
                    } else{
                        printf("No file name was entered.  Command aborted.\n");
                        break;
                    }
                }
                break;

/* "view" the multiplication table or the basis table on screen */
	    case 'v':
		 if(!Substr(Command, "view")){
                   printf("Illegal command.");
                   break;
		 }

		 table = Operand[0];			/* get the table type to show */

		 switch(table){
		   case 'b':
		     if(mtable_status == PRESENT){
		       PrintBasisTable(stdout);
		     }
		     else{
		       printf("Basis Table not present.\n");
		     }
		     break;
		   case 'm':
		     if(mtable_status == PRESENT){
		       Print_MultTable(stdout);
		     }
		     else{
		       printf("Multiplication Table not present.\n");
		     }
		     break;
		   default:
		     printf("Invalid table type, %c.\nTable type must be \"m\" or \"b\"\n", table);
		 }
		 break;

            case 't':

/* "type nonassoc. word". Find the type of nonassoc. word */

                 if (!Substr(Command,"type")) {
                     printf("Illegal command.");
                     break;
                 }
                 out_of_memory = FALSE;
                 Cur_poly = Create_poly(Operand,&Cur_poly_len,&out_of_memory);
                 if (Cur_poly == nullptr) {
                     if (!out_of_memory)
                         printf("%s is not a nonassociative word.",Operand);
                     break;
                 }
                 else if (Cur_poly->terms->next != nullptr)
                     printf("%s is not a nonassociative word.",Operand);
                 else {
                     i = Assoc_number(Cur_poly->terms->term);
                     printf("The association type of the word = %d.",i);
                 }
                 break;

            case 'h':

/* "help command". Get help. */

                 if (!Substr(Command,"help")) {
                     printf("Illegal command.");
                     break;
                 }
                 if (!Help(Operand))
                     printf("Illegal command.");
                 break;

            case 'q':

/* "quit". Quit Albert. */

                 if (!Substr(Command,"quit")) {
                     printf("Illegal command.");
                     break;
                 }
                 if (mtable_status == PRESENT) {
                     DestroyMultTable();
                     mtable_status = NOT_PRESENT;
                     printf("Destroyed the Multiplication Table.\n");
                 }
                 quit = 1;
                 break;

            case '\0':
                 break;

            default :
                 printf("Illegal command.");
                 break;
        }
   }
   DestroyTypeTable();
   DestroyMultTable();
   free(Command);
   free(Operand);

   return 0;
}

void Print_title()
{
    printf("\n\n         ((Albert)), Version 4.0, 2008\n"
           "Dept. of Computer Science, Clemson University\n"
           "\n"
           "Enhanced Version %s by kent.vandervelden@gmail.com\n"
           "Compiled " __DATE__ ", " __TIME__ "\n", enhanced_version);
#if defined(_OPENMP)
    printf("Parallel processing enabled: %d processors.\n\n\n", omp_get_num_procs());
#else
    printf("Parallel processing disabled.\n\n\n");
#endif
}

/* Called from S_init() of Scalar_arithmetic.c Used to build the inverse
 * table as a part of initialization. */

Scalar GetField()
{
    return Field;
}


/* struct P_type ptype is used to create Target type struct Type ttype.
 * If generators are 3a2b2c then Target type is 322. */

Type CreateTargetType(struct P_type ptype)
{
    int ttype_len = 1;
    int i,cur_tt_index = 0;
    Type ttype;

     for (i = 0;i<NUM_LETTERS;i++) {
         if (ptype.degrees[i] > 0) {
             ttype_len++;
         }
     }

     ttype = (Type) (Mymalloc(ttype_len * sizeof(Degree)));

     if (ttype != nullptr) {
         for (i = 0; i < NUM_LETTERS; i++) {
             if (ptype.degrees[i] > 0) {
                 ttype[cur_tt_index++] = ptype.degrees[i];
             }
         }
         ttype[cur_tt_index] = 0;
     }

     return ttype;
}


/* Check if the polynomial has the compatible problem type.
 * i.e if generators are 3a2b2c then each term in a polynomial should
 * have at most 3a's,2b's and 2c's. Else we can't use the multiplication
 * table to answer the query whether the poly is an identity or not. */

int Compatible(struct polynomial *Poly, struct P_type ptype)
{
    int i;

    if (Poly == nullptr)
        return 0;

    for (i=0;i<NUM_LETTERS;i++) {
        if (Poly->deg_letter[i] > ptype.degrees[i]) {
            return 0;
        }
    }

    return 1;
}


void usage()
{
    printf("Usage:  albert [-a dirname]\n");
}


void sigCatch(int)
{
  sigIntFlag = 1;
}

bool save_tables(const char *fn) {
    FILE *f = fopen(fn, "wb");
    if (!f) {
        printf("Unable to open %s for writing\n", fn);
        return false;
    }

    fwrite(&mult_file_version, sizeof(mult_file_version), 1, f);

    bool rv = save_mult_table(f) && save_basis_table(f) && save_type_table(f);

    fclose(f);

    return rv;
}

bool restore_tables(const char *fn) {
    FILE *f = fopen(fn, "rb");
    if (!f) {
        printf("Unable to open %s for reading\n", fn);
        return false;
    }

    unsigned short v;
    fread(&v, sizeof(v), 1, f);
    if(v != mult_file_version) {
        printf("Found incompatible file version while reading %s\n", fn);
        fclose(f);
        return false;
    }

    bool rv = restore_mult_table(f) && restore_basis_table(f) && restore_type_table(f);

    fclose(f);

    return rv;
}
